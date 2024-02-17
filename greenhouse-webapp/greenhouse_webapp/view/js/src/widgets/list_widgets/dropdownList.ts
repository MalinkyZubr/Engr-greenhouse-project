import { AbstractBaseWidgetHTMLController } from "../widget/dynamic/widget_html";
import { BaseStartupFieldParameters } from "../widget/dynamic/widget_html";
import { DropdownOptionController } from "../misc_inputs/dropdownOption";
import { ListAppendModule, WidgetListHTMLController } from "../widget/list_widget";


export abstract class DropdownListHTMLController extends WidgetListHTMLController {
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