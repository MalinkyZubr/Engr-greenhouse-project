import { ElementList, ListElement, request_server_list_data } from "./widgets/list/elementList.ts"
import { RequestButton } from "./widgets/buttons/requestButton.ts"
import { RadioButton } from "./widgets/buttons/radioButtons.ts"
import { host } from "./config.ts"
import { PeriodicExecutor } from "./shared/periodic.ts";
import { StandardWidget } from "./widgets/widget.ts";


class GraphManager extends StandardWidget {
    public widget_html: string = 
    `<div class="table wide-element" id={{ element_id }}>
        <h2>Project Data</h2>
        <img id="graph-image" src="">
    </div>`
    public widget_name = "GraphManager"

    private project_name: string;

    constructor(widget_data: object, parent_element: HTMLElement, widget_key: string, project_name: string) {
        super(widget_data, parent_element, widget_key);
        this.project_name = project_name;
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

    constructor(widget_data: object, parent_element: HTMLElement, widget_key: string, project_name: string) {
        super(widget_data, parent_element, widget_key);
        this.project_name = project_name;
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

class DataTypeRadio extends RadioButton {
    public widget_name: string = "DataTypeRadio";

    public set_static_value(value: string): void {
        DataTypeRadio.radio_button_group_value = value;
    }
}

class TimeframeRadio extends RadioButton {
    public widget_name: string = "TimeframeRadio";

    public set_static_value(value: string): void {
        TimeframeRadio.radio_button_group_value = value;
    }
}

class Device extends ListElement {
    public widget_name: string = "Device";
    public widget_html: string =
    `<tr>
        <td><button id="device_name"></button></td>
        <td><button id="device-status"></button></td>
    </tr>`;
}

class AssignedDevicesTable extends ElementList {
    public widget_name: string = "AssignedDevicesTable";

    public create_list_element(element_data: object, this_list_html: HTMLElement): ListElement {
        return new Device(element_data, this_list_html);
    }
}

class ProjectManager {
    private assigned_devices: AssignedDevicesTable;
    private timeframe_radios: TimeframeRadio;
    private datatype_radio: DataTypeRadio;
    private add_device_button: AddDeviceButton

    constructor() {

    }
}