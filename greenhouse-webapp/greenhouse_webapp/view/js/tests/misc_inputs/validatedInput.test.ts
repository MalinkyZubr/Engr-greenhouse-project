/**
 * @jest-environment jsdom
 */


import { ValidatedInputHTMLController, ValidatedInputStartupParameters } from "../../src/modules/widgets/misc_inputs/validatedInput";
import { BaseWidget, BaseWidgetMetadata, WidgetParent } from "../../src/modules/widgets/widget/widget";
import { WidgetListenerModule } from "../../src/modules/widgets/widget/widget";


let startup_parameters: ValidatedInputStartupParameters;
let html_controller: ValidatedInputHTMLController;
let containing_widget: BaseWidget;
let validation_module: WidgetListenerModule<ValidatedInputHTMLController>;

let dom_reference: HTMLElement
let dom_input_reference: HTMLInputElement;
let dom_error_reference: HTMLElement;

document.body.innerHTML = `<div id="parent"></div>`


startup_parameters = new ValidatedInputStartupParameters(
    "test_validator",
    "badingus",
    "The thingy is mad!!!!"
);

html_controller = new ValidatedInputHTMLController(startup_parameters, `^\\d\\s*\\d\\s*\\d$`);

validation_module = new WidgetListenerModule<ValidatedInputHTMLController>("change", "input")
    .inject_operation(async function(controller: ValidatedInputHTMLController): Promise<void> {
        controller.validate_input();
    });

containing_widget = new BaseWidget(html_controller,
    new BaseWidgetMetadata("test", new WidgetParent("parent")));

dom_reference = document.getElementById("test_validator") ?? function() {throw new Error("Test validator not found")}();
dom_input_reference = dom_reference.querySelector("#input") ?? function() {throw new Error("Test validator input not found")}();
dom_error_reference = dom_reference.querySelector("#error") ?? function() {throw new Error("Test validator error not found")}();

beforeAll(() => {

})

describe("Testing the functionality of the validated input widget html controller", () => {
    test("Testing to see that the controller rejects invalid input", () => {
        dom_input_reference.value = "silly billy is silly";

        html_controller.validate_input();

        expect(containing_widget.get_value).toThrow(Error);
        expect(dom_error_reference.style.display).toBe("none");
    });

    test("Testing to see that the controller accepts valid input", () => {
        dom_input_reference.value = "1 2 3";

        html_controller.validate_input();

        expect(containing_widget.get_value()).toStrictEqual({"test_validator":"1 2 3"});
        expect(dom_error_reference.style.display).toBe("block");
    })
})