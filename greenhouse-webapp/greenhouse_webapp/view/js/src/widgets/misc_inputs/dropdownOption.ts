import { WidgetListenerModule } from "../widget/widget";
import { AbstractBaseWidgetHTMLController, BaseStartupFieldParameters } from "../widget/dynamic/widget_html";
import { FieldParameters } from "../widget/dynamic/field_container";


export class DropdownOptionStartupFieldParameters extends BaseStartupFieldParameters {
    private dropdown_value: string;
    private tooltip: FieldParameters;


    constructor(element_id: string, dropdown_value: string, tooltip: FieldParameters) {
        super(element_id);
        this.dropdown_value = dropdown_value;
        this.tooltip = tooltip;
    }

    public get_dropdown_value(): string {
        return this.dropdown_value;
    }
}


export class DropdownOptionController extends AbstractBaseWidgetHTMLController<DropdownOptionStartupFieldParameters> {
    public html_template_generator(): string {
        return `<option id={{ element_id }} value={{ dropdown_value }} title={{ tooltip }}></option>`;
    }
}