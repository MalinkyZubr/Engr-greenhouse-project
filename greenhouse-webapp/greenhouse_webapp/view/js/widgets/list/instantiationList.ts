import { ObjectTemplateParameters, List, ListElement, request_server_list_data } from "./elementList.ts";
import { PeriodicExecutor } from "../../shared/periodic.ts";


try {
    let device_name = document.getElementById("device_name");
}
catch {
    let device_name = null;
}


export class InstantiationListElement extends ListElement {
    widget_name: string = "InstatiationListElement";
    widget_html: string = 
        `<tr id="{{ element_id }}">
            <td>
                <label class="radio-button" id="radio-label">
                    <input type="radio" name="options" value={{ element_id }}>
                    <span class="radiomark"></span>
                    {{ element_id }}
                </label>
            </td>
        </tr>`;

    private selection_group: NodeListOf<HTMLInputElement>;
    public static selection_group_value: string;

    constructor(list_element_data: object, parent_list: HTMLElement) {
        super(list_element_data, parent_list);

        this.selection_group = parent_list.querySelectorAll(`[name="options]`);
        this.get_widget_node().addEventListener("change", this.button_selection_callback)
    }

    private button_selection_callback() {
        for(var x = 0; x < this.selection_group.length; x++) {
            var radio_button: HTMLInputElement = this.selection_group[x];
            if(radio_button.checked) {
                InstantiationListElement.selection_group_value = radio_button.value;
            }
        }
    }
}

export abstract class InstantiationList extends List {
    abstract host: string;

    constructor(list_data: object, parent_element: HTMLElement, widget_key: string) { // assignment type is either preset_name or project_name
        super(list_data, parent_element, widget_key);
    }

    public async update_instantiation_list(): Promise<void> {
        var server_data: Map<string, object> | null = await request_server_list_data(this.host, "/");
        if(server_data) {
            this.handle_response(server_data);
        }
    }

    public create_list_element(element_data: object, this_list_html: HTMLElement): InstantiationListElement {
        return new InstantiationListElement(element_data, this_list_html);
    }

    public get_selected_value(): string {
        return InstantiationListElement.selection_group_value;
    }
}