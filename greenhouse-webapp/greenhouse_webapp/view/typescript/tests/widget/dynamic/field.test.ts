/**
 * @jest-environment jsdom
 */

import { HTMLControllerError } from "../../../src/modules/exceptions/module_errors";
import { DynamicField, StaticField } from "../../../src/modules/widgets/widget/dynamic/fields"


describe("Testing fields", () => {
    beforeEach(() => {
        document.body.innerHTML = `<div id=test></div>`;
    });

    describe('testing dyanmic fields', () => {
        test("Test that dynamic field can successfully set it's value", () => {
            var dynamic_field: DynamicField = new DynamicField(document.getElementById("test") ?? function() { throw new Error("") }());
            dynamic_field.set_value("test");
    
            expect(dynamic_field.get_value()).toBe("test");
            expect(document.getElementById("test")?.innerHTML).toBe("test");
        });
    })
    
    describe('testing static fields', () => {
        test("Test that static field can only be set once", () => {
            var static_field: StaticField = new StaticField(document.getElementById("test") ?? function() { throw new Error("") }());
            static_field.set_value("test1");
    
            expect(() => {static_field.set_value("test2")}).toThrow(HTMLControllerError);
            expect(static_field.get_value()).toBe("test1");
            expect(document.getElementById("test")?.innerHTML).toBe("test1");
        })
    })
});