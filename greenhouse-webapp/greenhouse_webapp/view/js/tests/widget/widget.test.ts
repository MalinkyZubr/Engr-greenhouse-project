/**
 * @jest-environment jsdom
 */

import { BaseStartupFieldParameters, AbstractBaseWidgetHTMLController } from "../../src/widgets/widget/dynamic/widget_html";
import { FieldParameters } from "../../src/widgets/widget/dynamic/field_container";
import { BaseWidget, RepetitiveModuleWrapper, WidgetModule, BaseWidgetMetadata, WidgetParent, WidgetRequestModule, BasicUpdaterRequestModule } from "../../src/widgets/widget/widget";
import { TestWidgetHTMLController, TestBasicUpdaterRequest } from "./test_classes";


document.body.innerHTML = `<div id="parent"></div>`

describe("Testing for the BaseWidget class", () => {
    var test_widget = new BaseWidget(
        new TestWidgetHTMLController(
            new BaseStartupFieldParameters("silly_test"),
        ),
        new BaseWidgetMetadata(
            "test_widget_name",
            new WidgetParent("parent")
        )
    )

    test("Testing all the upper layer getters", () => {
        expect(test_widget.get_name()).toBe("test_widget_name");
        expect(test_widget.get_id()).toBe("silly_test");
    })

    test("Testing the empty module", async () => {
        await expect(test_widget.run()).rejects.toThrow(EvalError);
    })

    var test_basic_updater_request: TestBasicUpdaterRequest = new TestBasicUpdaterRequest("/silly/zubr", 10);

    test("Testing the basic request module", () => {
        expect(test_basic_updater_request.generate_request_body()).toStrictEqual({method:"GET"});
    })

    test("Testing integration of basic request module with widget", async () => {
        test_widget.add_module(test_basic_updater_request);
        await test_widget.run();

        expect(test_widget.get_value()).toStrictEqual({"test_field1":"silly1", "test_field2":"silly2"});
        expect(document.getElementById("test_field2")?.innerHTML).toBe("silly2");
    })
})