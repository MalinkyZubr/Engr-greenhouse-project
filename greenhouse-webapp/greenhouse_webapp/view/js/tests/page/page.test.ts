/**
 * @jest-environment jsdom
 */


import { HTMLControllerError, WidgetError } from "../../src/modules/exceptions/module_errors";
import { Page } from "../../src/modules/page/page";


document.body.innerHTML = 
    `<div id="1" data-isfield="true"></div>
    <div id="2" data-isfield="true"></div>`

describe("Testing the functionality of the page class", () => {
    test("Testing to see if the page can successfully obtain all page fields, and allow consumption", () => {
        var test_page = new Page();
        expect(test_page.get_available_fields().size).toBe(2);
        expect(test_page.consume_element("1").id).toBe("1");
        expect(function() {test_page.consume_element("1")}).toThrow(WidgetError);
        expect(test_page.get_available_fields().size).toBe(1);
    });

    test("Testing to see if page can identify nonexistant fields", () => {
        var test_page = new Page();
        expect(function() {test_page.consume_element("3")}).toThrow(WidgetError);
    })

    test("Testing to see if the page identifies page fields without IDs", () => {
        document.body.innerHTML = `<div data-isfield="true"></div>
        <div id="2" data-isfield="true"></div>`
        expect(function() {new Page}).toThrow(HTMLControllerError);
    })
})
