import { ListAppendModule, WidgetListHTMLController } from "../widget/list_widget";
import { BaseStartupFieldParameters } from "../widget/dynamic/widget_html";


export class TableHTMLController extends WidgetListHTMLController<BaseStartupFieldParameters> {
    public html_template_generator(): string {
        return `
        <tbody id={{ element_id }}></tbody>`
    }
}