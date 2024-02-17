import { AbstractBaseWidgetHTMLController } from "../widget/dynamic/widget_html";
import { BaseStartupFieldParameters } from "../widget/dynamic/widget_html";


export class RadioButtonStartupFields extends BaseStartupFieldParameters {
    private radio_group_name: string;
    private radio_value: string;
    
    constructor(element_id: string, radio_group_name: string, radio_value: string = "") {
        super(element_id)
        this.radio_group_name = radio_group_name;
        this.radio_value = radio_value;
    }

    public get_radio_group_name(): string {
        return this.radio_group_name;
    }
}


export class RadioButton extends AbstractBaseWidgetHTMLController<RadioButtonStartupFields> {
    public html_template_generator(): string {
        return `<input type="radio" id={{ element_id }} name={{ radio_group_name }}, value={{ radio_value }}>
        <label for={{ element_id }} data-field="dynamic"></label><br>`;
    }

    public get_radio_group_name(): string {
        return this.get_startup_fields().get_radio_group_name();
    }
}

    
