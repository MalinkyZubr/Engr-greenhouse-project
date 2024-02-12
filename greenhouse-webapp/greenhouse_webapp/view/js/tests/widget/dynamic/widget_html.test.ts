/**
 * @jest-environment jsdom
 */

import { BaseStartupFieldParameters, AbstractBaseWidgetHTMLController } from "../../../src/widgets/widget/dynamic/widget_html";
import { FieldParameters } from "../../../src/widgets/widget/dynamic/field_container";
import { TestWidgetHTMLController } from "../test_classes"


class ErroneousStartupFieldParameters extends BaseStartupFieldParameters {
    private silly: string;

    constructor(element_id: string, silly: string) {
        super(element_id);
        this.silly = silly;
    }
}

class ErroneousTestHTML extends AbstractBaseWidgetHTMLController<BaseStartupFieldParameters> {
    public html_template_generator(): string {
        return `<div id="dinkle"></div>`
    }
}

document.body.innerHTML = `<div id="parent"></div>`
var startup_field_values = new BaseStartupFieldParameters("test_id");
var test_html_controller = new TestWidgetHTMLController(startup_field_values);


test_html_controller.assign_widget_node(document.getElementById("parent") ?? function() { throw new Error("parent not found")}()); // this occurs inside widget class, but that is not being tested here

describe("Testing the functionality of BaseStartupFieldParameters and WidgetHTML", () => {
    test("Testing to see if the startup_fields integrated with the widget_html", () => {
        expect(startup_field_values.get_element_id()).toBe("test_id");
        expect(document.body.innerHTML.includes('id="test_id"'))
    });

    test("Testing to see if the HTML controller child getter works properly", () => {
        expect(test_html_controller.extract_child("test_child").innerHTML).toBe("this is a test child");
    });

    test("Testing to see if the HTML id integrated properly", () => {
        expect(test_html_controller.get_id()).toBe("test_id");
    });

    test("Testing to see if dynamic fields set properly through the html object", () => {
        test_html_controller.update_dynamic_fields({"test_field1":"Mugwump", "test_field2":"Stalwart"});

        expect(test_html_controller.get_value()["test_field1"]).toBe("Mugwump");
        expect(test_html_controller.extract_child("test_field1").innerHTML).toBe("Mugwump");

        expect(test_html_controller.get_value()["test_field2"]).toBe("Stalwart");
        expect(test_html_controller.extract_child("test_field2").innerHTML).toBe("Stalwart");
    })

    test("Testing to see if startup fields recognize erroneous startup keys", () => {
        expect(function() {new TestWidgetHTMLController(new ErroneousStartupFieldParameters("test1", "sillyerror"))}).toThrow(Error);
    })

    test("Testing to see if the widget HTML detects improper HTML formatting", () => {
        expect(function() {new ErroneousTestHTML(startup_field_values)}).toThrow(Error);
    })
})