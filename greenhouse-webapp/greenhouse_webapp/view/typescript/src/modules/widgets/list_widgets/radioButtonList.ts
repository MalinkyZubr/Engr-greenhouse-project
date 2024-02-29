import { BaseWidget } from "../widget/widget";
import { RadioButton, RadioButtonStartupFields } from "../buttons/radioButton";
import { WidgetListHTMLController, ListAppendModule } from "../widget/list_widget";
import { BaseStartupFieldParameters } from "../widget/dynamic/widget_html";
import { FieldParameters } from "../widget/dynamic/field_container";
import { HTMLControllerError } from "../../exceptions/module_errors";


export class RadioButtonListHTMLController extends WidgetListHTMLController<RadioButtonStartupFields> {
    public html_template_generator(): string {
        return `<div id={{ element_id }} data-radio-group={{ group_name }}></div>`;
    }

    public get_value(): FieldParameters {
        var selected_field: HTMLInputElement | null = this.get_node().querySelector('input[name="options"]:checked');

        if(!selected_field) {
            throw new HTMLControllerError("FIELDS_ERROR", 
                `No radio button has been selected!`, this);
        }

        return {"value":selected_field.value};
    }
}


export abstract class RadioButtonListAppendModule extends ListAppendModule<RadioButton> {
    private radio_button_group: string;

    constructor(request_route: string, radio_button_group: string) {
        super(request_route);
        this.radio_button_group = radio_button_group;
    }

    public generate_html(element_id: string, element_fields: FieldParameters): RadioButton {
        var radio_button_html: RadioButton = new RadioButton(
            new RadioButtonStartupFields(
                element_id,
                this.radio_button_group
            )
        );
        return radio_button_html;
    }
}

