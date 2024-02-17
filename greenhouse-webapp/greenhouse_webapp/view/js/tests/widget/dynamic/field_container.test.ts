/**
 * @jest-environment jsdom
 */

import { DynamicFields, StaticFields } from "../../../src/widgets/widget/dynamic/field_container"
import type { FieldParameters } from "../../../src/widgets/widget/dynamic/field_container";


let test_html: string;

test_html = 
        `<div id=container>
            <div id=x data-field="dynamic"></div>
            <div id=y data-field="dynamic"></div>
            <div id=z data-field="static"></div>
            <div id=a data-field="static"></div>
        </div>`;
document.body.innerHTML = test_html;

describe("Testing field containers", () => {
    describe("Testing DynamicFields container type", () => {
        var testing_fields: DynamicFields = new DynamicFields(document.getElementById("container") ?? function(){throw new Error("THE SELECTED ID WAS NOT FOUND")}());
    
        test("Testing DynamicFields constructor", () => {
            expect(testing_fields.get_field_values()).toStrictEqual({"x":"", "y":""})
        });
        
        test("Testing whether values set correctly for DynamicFields", () => {
            testing_fields.set_field_values({"x":"test1", "y":"test2"});
            expect(testing_fields.get_field_values()["y"]).toBe("test2");
            expect(document.getElementById("y")?.innerHTML).toBe("test2");
        });
    })
    
    describe("Testing StaticFields container type", () => {
        var testing_fields: StaticFields = new StaticFields(document.getElementById("container") ?? function(){throw new Error("THE SELECTED ID WAS NOT FOUND")}());
    
        test("Testing StaticFields constructor", () => {
            expect(testing_fields.get_field_values()).toStrictEqual({"z":"", "a":""});
        })
    
        test("Testing whether values are set correctly for StaticFields", () => {
            testing_fields.set_field_values({"z": "test3", "a":"test5"});
            expect(testing_fields.get_field_values()["z"]).toBe("test3");
            expect(document.getElementById("z")?.innerHTML).toBe("test3");
        })
    
        test("Testing whether static fields are only settable once", () => {
            expect(() => {testing_fields.set_field_values({"z":"test4"})}).toThrow(TypeError);
        })
    })
})