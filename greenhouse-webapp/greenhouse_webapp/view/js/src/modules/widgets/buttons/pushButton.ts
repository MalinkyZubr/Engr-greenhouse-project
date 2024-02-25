import { AbstractBaseWidgetHTMLController } from "../widget/dynamic/widget_html";
import { BaseStartupFieldParameters } from "../widget/dynamic/widget_html";


export class PushButton extends AbstractBaseWidgetHTMLController<BaseStartupFieldParameters> {
    public html_template_generator(): string {
        return `<button class="button" id="{{ element_id }}" data-field="dynamic"></button>`;
    }
}

