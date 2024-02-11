import { BaseStartupFieldParameters, AbstractBaseWidgetHTMLController } from "../../src/widgets/widget/dynamic/widget_html";
import { FieldParameters } from "../../src/widgets/widget/dynamic/field_container";


export class TestWidgetHTMLController extends AbstractBaseWidgetHTMLController<BaseStartupFieldParameters> {
    public html_template_generator(): string {
        return `
        <div id={{ element_id }}>
            Hi there, im a test controller!
            <div id="test_child">this is a test child</div>
            <div id="test_field1" data-field="dynamic">GooberToober</div>
            <div id="test_field2" data-field="dynamic">TooberGoober</div>
        </div>\n`;
    }
}