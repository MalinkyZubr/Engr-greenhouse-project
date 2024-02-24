import { BaseStartupFieldParameters, AbstractBaseWidgetHTMLController } from "../../src/modules/widgets/widget/dynamic/widget_html";
import { FieldParameters } from "../../src/modules/widgets/widget/dynamic/field_container";
import { WidgetRequestModule, BasicUpdaterRequestModule } from "../../src/modules/widgets/widget/widget";


export class TestWidgetHTMLController extends AbstractBaseWidgetHTMLController<BaseStartupFieldParameters> {
    public html_template_generator(): string {
        return `
        
        <div id={{ element_id }}>
            Hi there, im a test controller!
            <div id="test_child">this is a test child</div>
            <div id="test_field1" data-field="dynamic">GooberToober</div>
            <div id="test_field2" data-field="dynamic">TooberGoober</div>
        </div>\n\n`;
    }
}


export class TestBasicUpdaterRequest extends BasicUpdaterRequestModule {
    public async submit_request(): Promise<void> {
        var res: FieldParameters = {"test_field1":"silly1", "test_field2":"silly2"};
        this.process_response(res);
    }
}


