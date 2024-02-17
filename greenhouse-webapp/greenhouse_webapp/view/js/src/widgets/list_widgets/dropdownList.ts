import { AbstractBaseWidgetHTMLController } from "../widget/dynamic/widget_html";
import { BaseStartupFieldParameters } from "../widget/dynamic/widget_html";


export abstract class DropdownOption extends ListElement {
    abstract widget_html: string;
    abstract widget_name: string;

    public static generate_tooltip(tooltip_data: object) {
        var tooltip_string: String = "";
        for(const [key, value] of Object.entries(tooltip_data)) {
            tooltip_string = tooltip_string.concat(`${key}: ${value}\n`);
        }
        return tooltip_string;
    }
}

export class StandardDropdownOption extends DropdownOption {
    widget_html: string = `<option id={{ element_id }} value={{ value }} title={{ tooltip }}></option>`
    widget_name: string = "DropdownOption"
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

    constructor(widget_data: object, parent_element: WidgetParentData) {
        super(widget_data, parent_element);
        this.dropdown_element = this.get_widget_node().querySelector("#dropdown") ?? function() { throw new Error("dropdown not found"); }();
        this.dropdown_element.addEventListener("change", this.selection_callback);
    }

    abstract create_list_element(element_data: object): ListElement;

    private async selection_callback(): Promise<void> {
        this.selected_value = this.dropdown_element.value;
    }

    public get_selected_value(): string {
        return this.selected_value;
    }
}