import { ElementList, ListElement, request_server_list_data } from "./widgets/list/elementList.ts"
import { RequestButton } from "./widgets/buttons/requestButton.ts"
import { RadioButton, RadioGroup } from "./widgets/buttons/radioButtons.ts"
import { host } from "./config.ts"
import { PeriodicExecutor } from "./shared/periodic.ts";
import { WidgetParentData, Widget } from "./widgets/widget.ts";
import { ValidatedInput } from "./widgets/validated_input/validatedInput.ts";


class ProjectParentData extends WidgetParentData {
    private project_name: string;

    constructor(parent_id: string, project_name: string) {
        super(parent_id);
        this.project_name = project_name;
    }

    public get_project_name(): string {
        return this.project_name;
    }
}

class GraphManager extends Widget {
    public widget_html: string = 
    `<div class="table wide-element" id={{ element_id }}>
        <h2>Project Data</h2>
        <img id="graph-image" src="">
    </div>`
    public widget_name = "GraphManager"

    private project_name: string;
    private project_page: ProjectPage;

    constructor(widget_data: object, parent_element: ProjectParentData, project_page: ProjectPage) {
        super(widget_data, parent_element);
        this.project_name = parent_element.get_project_name();
    }

    public async get_graph_image(interval: [string, string] = ["", ""], data_type: string): Promise<void> {
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

    public async run(): Promise<void> {
        if(this.project_page.get)
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
}

class TimeframeRadio extends RadioGroup {
    public widget_name: string = "TimeframeRadio";
}

class Device extends ListElement { // update it so it stores a button widget
    public widget_name: string = "Device";
    public widget_html: string =
    `<tr>
        <td><button id="device_name"></button></td>
        <td><button id="device-status"></button></td>
    </tr>`;
    private project_name;

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
        return new Device(element_data, new WidgetParentData(this.get_id()), this.project_name);
    }
}

class RangeInput extends ValidatedInput {
    format_regex: RegExp = new RegExp('(?!00)\\d{2}-(?!00)\\d{2}-(?!00)\\d{2}');
}

class ProjectPage{
    private project_name: string;

    private device_table: AssignedDevicesTable = new AssignedDevicesTable(
        {
            element_id: "assigned_devices_table",
            list_name: "Assigned Devices",
            table_header: "<th>Device</th>\n<th>Status</th>"
        },
        new ProjectParentData("parent_container", 
            this.get_project_name())
        );

    private timeframe_radio: TimeframeRadio = new TimeframeRadio(
        {
            element_id: "timeframe_radio",
            radio_group_name: "timeframe_group",
        },
        new WidgetParentData("data-view-type"),
    )
        .add("live")
        .add("selected");

    private datatype_radio: DataTypeRadio = new DataTypeRadio(
        {
            element_id: "datatype_radio",
            radio_group_name: "datatype_group",
        },
        new WidgetParentData("datatype-selector")
    )
        .add("Temperature")
        .add("Humidity")
        .add("Moisture")
        .add("Light");

    private archive_button: ArchiveProjectButton = new ArchiveProjectButton(
        {
            element_id: "archive",
            button_name: "Archive Project"
        },
        new ProjectParentData("archive_button", this.get_project_name()),
        this
    );

    private download_csv_button: CsvDownloadButton = new CsvDownloadButton(
        {
            element_id: "csv_download",
            button_name: "Download CSV"
        },
        new ProjectParentData("download_csv",
            this.get_project_name()),
        this
    )

    private graph_data_image: GraphManager = new GraphManager(
        {
            element_id: "graph_manager"
        },
        new ProjectParentData("graph_image",
            this.get_project_name()),
        this
    )

    private start_date_input: RangeInput = new RangeInput(
        {
            element_id: "start_date",
            placeholder: "mm-dd-yy",
            error_message: "Must be mm-dd-yy"
        },
        new WidgetParentData("start_date_input"),
    )

    private end_date_input: RangeInput = new RangeInput(
        {
            element_id: "end_date",
            placeholder: "mm-dd-yy",
            error_message: "Must be mm-dd-yy"
        },
        new WidgetParentData("end_date_input")
    )

    constructor() {
        this.project_name = document.getElementById("project_name")?.textContent ?? function() { throw new Error("project name not found"); }();
    }

    public get_project_name() {
        return this.project_name;
    }
}


function main() {
    var page: ProjectPage = new ProjectPage()
}


document.addEventListener("DOMContentLoaded", main);