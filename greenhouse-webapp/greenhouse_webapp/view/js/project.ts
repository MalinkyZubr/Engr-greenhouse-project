import { ElementList, ListElement, request_server_list_data } from "./widgets/list/elementList.ts"
import { RequestButton } from "./widgets/buttons/requestButton.ts"
import { RadioButton, RadioGroup } from "./widgets/buttons/radioButtons.ts"
import { host } from "./config.ts"
import { PeriodicExecutor } from "./shared/periodic.ts";
import { StandardWidget, DynamicPage, WidgetParentData, WidgetParentDataStrict } from "./widgets/widget.ts";
import { ValidatedInput } from "./widgets/validated_input/validatedInput.ts";


class ProjectParentData extends WidgetParentDataStrict {
    private project_name: string;

    constructor(parent_element: HTMLElement, project_name: string, widget_key: string) {
        super(parent_element, widget_key);
        this.project_name = project_name;
    }

    public get_project_name(): string {
        return this.project_name;
    }
}

class GraphManager extends StandardWidget {
    public widget_html: string = 
    `<div class="table wide-element" id={{ element_id }}>
        <h2>Project Data</h2>
        <img id="graph-image" src="">
    </div>`
    public widget_name = "GraphManager"

    private project_name: string;

    constructor(widget_data: object, parent_element: ProjectParentData) {
        super(widget_data, parent_element);
        this.project_name = parent_element.get_project_name();
    }

    async get_graph_image(interval: [string, string] = ["", ""], data_type: string): Promise<void> {
        await fetch(`/projects/projects/${this.project_name}/data_visualization/${data_type}`,
        {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({
                start_date: interval[0],
                end_date: interval[1]
            })
        })
        .then(response => response.blob())
        .then(blob => {
            const image_url: string = URL.createObjectURL(blob);
            var image_element: HTMLImageElement = this.get_widget_node().querySelector("#graph-image") ?? function() { throw new Error("graph image not found"); }();
            image_element.src = image_url;
        });
    }
}

abstract class ProjectManagerButton extends RequestButton {
    private project_name: string;
    private project_page: ProjectPage;

    constructor(widget_data: object, parent_element: ProjectParentData, project_page: ProjectPage) {
        super(widget_data, parent_element);
        this.project_name = parent_element.get_project_name();
        this.project_page = project_page;
    }

    public get_project_name(): string {
        return this.project_name;
    }
}

class CsvDownloadButton extends ProjectManagerButton {
    public async submit_button_request(): Promise<void> {
        var download_link: HTMLAnchorElement = document.createElement('a');
        download_link.href = `/projects/projects/${this.get_project_name()}/download_csv`;
        download_link.download = `${this.get_project_name()}.csv`;

        document.body.appendChild(download_link);
        download_link.click();
        document.body.removeChild(download_link);
    }
}

class ArchiveProjectButton extends ProjectManagerButton {
    public async submit_button_request(): Promise<void> {
        var confirmation = prompt(`You must confirm archiving of this project\nEnter the project name: ${this.get_project_name()}`);

        if (confirmation === this.get_project_name()) {
            await fetch(`http://localhost/projects/projects/${this.get_project_name()}/archive`, { // need port in route
                method: 'PATCH'
            })
        }
        else {
            alert("Archiving cancelled!")
        }
    }
}

class AddDeviceButton extends ProjectManagerButton {
    public async submit_button_request(): Promise<void> {
        // must fetch a new route that supplies scans and idles
    }
}

class DataTypeRadio extends RadioGroup {
    public widget_name: string = "DataTypeRadio";

    public set_static_value(value: string): void {
        DataTypeRadio.radio_button_group_value = value;
    }
}

class TimeframeRadio extends RadioGroup {
    public widget_name: string = "TimeframeRadio";

    public set_static_value(value: string): void {
        TimeframeRadio.radio_button_group_value = value;
    }
}

class Device extends ListElement { // update it so it stores a button widget
    public widget_name: string = "Device";
    public widget_html: string =
    `<tr>
        <td><button id="device_name"></button></td>
        <td><button id="device-status"></button></td>
    </tr>`;
    private project_name

    constructor(list_element_data: object, parent_list: WidgetParentData, project_name: string) {
        super(list_element_data, parent_list)
    }
}

class AssignedDevicesTable extends ElementList {
    public widget_name: string = "AssignedDevicesTable";
    private project_name: string;

    constructor(list_data: object, parent_element: ProjectParentData) {
        super(list_data, parent_element);
        this.project_name = parent_element.get_project_name();
    }

    public create_list_element(element_data: object): ListElement {
        return new Device(element_data, new WidgetParentData(this.get_widget_node()), this.project_name);
    }
}

class RangeInput extends ValidatedInput {
    format_regex: RegExp = new RegExp('(?!00)\\d{2}-(?!00)\\d{2}-(?!00)\\d{2}');
}

class ProjectPage extends DynamicPage {
    private project_name: string;
    constructor() {
        super();
        this.project_name = document.getElementById("project_name")?.textContent ?? function() { throw new Error("project name not found"); }();
    }

    public get_project_name() {
        return this.project_name;
    }
}


function main() {
    var page: ProjectPage = new ProjectPage();

    var buttons: HTMLElement = document.getElementById("buttons") ?? function() { throw new Error("buttons not found") }()
    var parent_container: HTMLElement = document.getElementById("parent_container") ?? function() { throw new Error("parent_container not found") }();
    var range_input: HTMLElement = document.getElementById("display-interval") ?? function() { throw new Error("display-interval not found") }();

    var timeframe_radio = new TimeframeRadio(
        {
            element_id: "timeframe_radio",
            radio_group_name: "timeframe_group",
        },
        new WidgetParentDataStrict(document.getElementById("data-view-type") ?? function() { throw new Error("data-view-type not found") }(),
            "timeframe_radio"),
    )
        .add("live")
        .add("selected");
    
    var datatype_radio = new DataTypeRadio(
        {
            element_id: "datatype_radio",
            radio_group_name: "datatype_group",
        },
        new WidgetParentDataStrict(document.getElementById("datatype-selector") ?? function() { throw new Error("data-type-selector not found") }(), 
            "datatype_radio")
    )
        .add("Temperature")
        .add("Humidity")
        .add("Moisture")
        .add("Light");

    var archive_button: ArchiveProjectButton = new ArchiveProjectButton(
        {
            element_id: "archive",
            button_name: "Archive Project"
        },
        new ProjectParentData(buttons, 
            page.get_project_name(),
            "archive_button"),
        page
    );

    var download_csv_button: CsvDownloadButton = new CsvDownloadButton(
        {
            element_id: "csv_download",
            button_name: "Download CSV"
        },
        new ProjectParentData(buttons,
            page.get_project_name(),
            "download_csv"),
        page
    )

    // the add device should be a dropdown instead

    var graph_data_image: GraphManager = new GraphManager(
        {
            element_id: "graph_manager"
        },
        new ProjectParentData(parent_container,
            "graph_image",
            page.get_project_name())
    )

    var device_table = new AssignedDevicesTable(
        {
            element_id: "assigned_devices_table",
            list_name: "Assigned Devices",
            table_header: "<th>Device</th>\n<th>Status</th>"
        },
        new ProjectParentData(parent_container, 
            page.get_project_name(),
            "assigned_devices_table")
        );

    var start_date_input: RangeInput = new RangeInput(
        {
            element_id: "start_date",
            placeholder: "mm-dd-yy",
            error_message: "Must be mm-dd-yy"
        },
        new WidgetParentDataStrict(range_input, "start_date_input"),
    )

    var end_date_input: RangeInput = new RangeInput(
        {
            element_id: "end_date",
            placeholder: "mm-dd-yy",
            error_message: "Must be mm-dd-yy"
        },
        new WidgetParentDataStrict(range_input, "end_date_input"),
    )

    page // make sure to add the add_device buttons
        .add(device_table)
        .add(timeframe_radio)
        .add(datatype_radio)
        .add(start_date_input)
        .add(end_date_input)
        .add(graph_data_image)
        .add(download_csv_button)
        .add(archive_button);
}


document.addEventListener("DOMContentLoaded", main);