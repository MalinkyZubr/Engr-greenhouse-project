/**
 * @jest-environment jsdom
 */

import { BaseStartupFieldParameters, AbstractBaseWidgetHTMLController } from "../../src/widgets/widget/dynamic/widget_html";
import { FieldParameters } from "../../src/widgets/widget/dynamic/field_container";
import { BaseWidget, RepetitiveModuleWrapper, WidgetModule, BaseWidgetMetadata, WidgetParent, WidgetRequestModule } from "../../src/widgets/widget/widget";
import { TestWidgetHTMLController } from "./test_classes";


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

    test("Testing the empty module", () => {
        expect(test_widget.run()).toThrow(EvalError);
    })

    test("Testing the request module", () => {
        
    })
})