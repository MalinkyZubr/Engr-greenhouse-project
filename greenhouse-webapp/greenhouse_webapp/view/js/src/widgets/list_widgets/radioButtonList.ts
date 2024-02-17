import { BaseWidget } from "../widget/widget";
import { RadioButton, RadioButtonStartupFields } from "../buttons/radioButton";
import { WidgetListHTMLController, ListAppendModule } from "../widget/list_widget";
import { BaseStartupFieldParameters } from "../widget/dynamic/widget_html";


export class RadioButtonListHTMLController extends WidgetListHTMLController<RadioButtonStartupFields> {
    public html_template_generator(): string {
        return `<div id={{ element_id }} data-radio-group={{ group_name }}></div>`;
    }
}


export abstract class RadioButtonListAppendModule extends ListAppendModule<RadioButton> {
    private radio_button_group: string;

    constructor(request_route: string, request_interval: number, radio_button_group: string) {
        super(request_route, request_interval);
        this.radio_button_group = radio_button_group;
    }

    public generate_html(element_id: string): RadioButton {
        var radio_button_html: RadioButton = new RadioButton(
            new RadioButtonStartupFields(
                element_id,
                this.radio_button_group
            ));
        return radio_button_html;
    }
}

