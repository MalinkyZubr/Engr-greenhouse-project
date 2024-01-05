import { StandardWidget, Widget } from "../widget";
import { ListElement, List } from "./elementList";


export class DropdownOption extends ListElement {
    widget_html: string = `<option id={{ element_id }} value={{ value }} title={{ tooltip }}></option>`
    widget_name: string = "DropdownOption"

    public static generate_tooltip(tooltip_data: object) {
        var tooltip_string: String = "";
        for(const [key, value] of Object.entries(tooltip_data)) {
            tooltip_string = tooltip_string.concat(`${key}: ${value}\n`);
        }
        return tooltip_string;
    }
}

export abstract class Dropdown extends List {
    abstract widget_name: string;
    public widget_html: string = 
    `<div id={{ element_id }}>
        {{ dropdown_name }}
        <select id="dropdown">
        </select>
    </div>`

    public table_element_id: string = "dropdown";
    private selected_value: string;
    private dropdown_element: HTMLSelectElement;

    constructor(widget_data: object, parent_element: HTMLElement, widget_key: string) {
        super(widget_data, parent_element, widget_key);
        this.dropdown_element = this.get_widget_node().querySelector("#dropdown") ?? function() { throw new Error("dropdown not found"); }();
        this.dropdown_element.addEventListener("change", this.selection_callback);
    }

    public create_list_element(element_data: object, this_list_html: HTMLElement): ListElement {
        return new DropdownOption(element_data, this_list_html);
    }

    private async selection_callback(): Promise<void> {
        this.selected_value = this.dropdown_element.value;
    }

    public get_selected_value(): string {
        return this.selected_value;
    }
}