/**
 * @jest-environment jsdom
 */


import { WidgetListHTMLController, ListAppendModule, FieldAction } from "../../src/modules/widgets/widget/list_widget";
import { BaseWidget, BaseWidgetMetadata, WidgetParent } from "../../src/modules/widgets/widget/widget";
import { BaseStartupFieldParameters } from "../../src/modules/widgets/widget/dynamic/widget_html";
import { TestWidgetHTMLController } from "./test_classes";
import { FieldParameters, ListParameters } from "../../src/modules/widgets/widget/dynamic/field_container";


class TestList extends WidgetListHTMLController<BaseStartupFieldParameters> {
    public html_template_generator(): string {
        return `<div id={{ element_id }}></div>`
    }
}

class TestListAppendModule extends ListAppendModule<TestWidgetHTMLController> {
    public generate_html(element_id: string, element_fields: FieldParameters): TestWidgetHTMLController {
        return new TestWidgetHTMLController(new BaseStartupFieldParameters(element_id));
    }

    public generate_request_body(): object {
        return {"silly":"zubr25"}
    }
}

document.body.innerHTML = "<div id='parent'></div>";


let test_response1: ListParameters = {
    "SZ25":{
        "test_field1":"Im SillyZubr25!", 
        "test_field2":"...gdybyś nie wiedział!"
    },
    "TQ":{
        "test_field1":"Im the TetanusQueen!",
        "test_field2":"Palita kog libro, unya palit kog libro, tanga!"
    },
    "Gumbo":{
        "test_field1":"NaN",
        "test_field2":"NaN"
    }
}

let test_response2: ListParameters = {
    "SZ25":{
        "test_field1":"Im SillyZubr25!", 
        "test_field2":"...i całą moją miłość kieruję do królowej tężca!!"
    },
    "TQ":{
        "test_field1":"Im the TetanusQueen!",
        "test_field2":"Palita kog libro, unya palit kog libro, tanga!"
    },
}

let test_list_widget_html = new TestList(new BaseStartupFieldParameters("test_list"));
let test_module = new TestListAppendModule("/")
let containing_widget: BaseWidget = new BaseWidget(test_list_widget_html, 
    new BaseWidgetMetadata("silly_list", 
        new WidgetParent("parent")));
containing_widget.add_core_module(test_module)


describe("Testing the functionality of the base list widget", () => {
    describe("Testing the widget list without integrating the module", () => {
        let test_member_widget: BaseWidget = new BaseWidget(
            new TestWidgetHTMLController(
                new BaseStartupFieldParameters("SillyZubr")
            ),
            new BaseWidgetMetadata("test_widget",
                new WidgetParent(test_list_widget_html.get_append_node())
            )
        );

        var test_child_parameters = {"test_field1":"Hello", "test_field2":"world"}

        test("testing to see if the list html controller", () => {
            expect(function() { new TestList(new BaseStartupFieldParameters("test_list"), "dinkleberry")}).toThrow(Error)
        });

        test("testing to see if the append node for the list defaulted to the dynamic ID", () => {
            expect(test_list_widget_html.get_append_node().querySelector("#silly_list")).toBe(document.getElementById("silly_list"));
        });

        test("testing the function determine if it identifies 'CREATE' action", () => {
            expect(test_list_widget_html.determine_creationary_action("mugwump", test_child_parameters)).toBe(FieldAction.Create);
        });

        test("testing to see if the add_child method works correctly", () => {
            test_list_widget_html.add_child(test_member_widget);
            expect(test_list_widget_html.get_child("SillyZubr").get_id()).toBe("SillyZubr");
            expect(document.getElementById("SillyZubr") != null).toBe(true);
            expect(function() { test_list_widget_html.get_child("dingus") }).toThrow(Error);
        });

        test("test creationary action on already created member", () => {
            expect(test_list_widget_html.determine_creationary_action("SillyZubr", test_child_parameters)).toBe(FieldAction.Update);
        })

        test("testing to see that the widget children update properly", () => {
            test_list_widget_html.update_child("SillyZubr", test_child_parameters);
            expect(test_list_widget_html.determine_creationary_action("SillyZubr", test_child_parameters)).toBe(FieldAction.NoChange);
        })

        test("testing to see if the controller can successfully remove a child widget", () => {
            test_list_widget_html.delete_necessary(new Array<string>());
            expect(function() { test_list_widget_html.get_child("SillyZubr") }).toThrow(Error);
        })
    });
    
    describe("Testing the integration of the list module to the list controller", () => {
        test("testing to see if the module can load multiple child widgets at once", () => {
            test_module.process_response(test_response1);
            expect(test_list_widget_html.get_child("SZ25").get_id()).toBe("SZ25");
            expect(test_list_widget_html.get_child("TQ").get_id()).toBe("TQ");
            expect(test_list_widget_html.get_child("Gumbo").get_id()).toBe("Gumbo");

            expect(document.getElementById("SZ25")?.querySelector("#test_field1")?.innerHTML).toBe("Im SillyZubr25!");
            expect(document.getElementById("TQ")?.querySelector("#test_field1")?.innerHTML).toBe("Im the TetanusQueen!");
        });

        test("Testing to see if modification and deletion works through the module", () => {
            test_module.process_response(test_response2);
            expect(function() {test_list_widget_html.get_child("Gumbo").get_id() }).toThrow(Error);
            expect(document.getElementById("SZ25")?.querySelector("#test_field2")?.innerHTML).toBe("...i całą moją miłość kieruję do królowej tężca!!");
        })
    })
})



