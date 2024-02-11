/**
 * @jest-environment jsdom
 */

import { DynamicField, StaticField } from "../../../src/widgets/widget/dynamic/fields"


describe('testing dyanmic fields', () => {
    test("Test that dynamic field can successfully set it's value", () => {
        document.body.innerHTML = `<div id=test></div>`;
        var dynamic_field: DynamicField = new DynamicField(document.getElementById("test") ?? function() { throw new Error("") }());
        dynamic_field.set_value("test");

        expect(dynamic_field.get_value()).toBe("test");
        expect(document.getElementById("test")?.innerHTML).toBe("test");
    });
})

describe('testing static fields', () => {
    test("Test that static field can only be set once", () => {
        document.body.innerHTML = `<div id=test></div>`;
        var static_field: StaticField = new StaticField(document.getElementById("test") ?? function() { throw new Error("") }());
        static_field.set_value("test1");

        expect(() => {static_field.set_value("test2")}).toThrow(TypeError);
        expect(static_field.get_value()).toBe("test1");
        expect(document.getElementById("test")?.innerHTML).toBe("test1");
    })
})