/**
 * @jest-environment jsdom
 */

import { BaseStartupFieldParameters, AbstractBaseWidgetHTMLController } from "../../src/modules/widgets/widget/dynamic/widget_html";
import { FieldParameters } from "../../src/modules/widgets/widget/dynamic/field_container";
import { BaseWidget, RepetitiveModuleWrapper, WidgetModule, BaseWidgetMetadata, WidgetParent, WidgetRequestModule, BasicUpdaterRequestModule, WidgetListenerModule } from "../../src/modules/widgets/widget/widget";
import { TestWidgetHTMLController, TestBasicUpdaterRequest } from "./test_classes";


class TestButtonStartupFields extends BaseStartupFieldParameters {
    private button_name: string;

    constructor(element_id: string, button_name: string) {
        super(element_id);
        this.button_name = button_name
    }
}

class TestButtonHTMLController extends AbstractBaseWidgetHTMLController<TestButtonStartupFields> {
    public html_template_generator(): string {
        return `<button class="button" id={{ element_id }} data-field="dynamic">{{ button_name }}</button>`
    }
}

class RepetitiveModuleTest extends WidgetModule {
    private count: number;

    constructor() {
        super();
        this.count = 0;
    }

    public async run(): Promise<void> {
        this.count += 1;
    }

    public get_run_count(): number {
        return this.count;
    }
}

let test_widget: BaseWidget;
let test_basic_updater_request: TestBasicUpdaterRequest;

document.body.innerHTML = `<div id="parent"></div>`

beforeAll(() => {
    test_widget = new BaseWidget(
        new TestWidgetHTMLController(
            new BaseStartupFieldParameters("silly_test"),
        ),
        new BaseWidgetMetadata(
            "test_widget_name",
            new WidgetParent("parent")
        )
    );
    test_basic_updater_request = new TestBasicUpdaterRequest("/silly/zubr");
});


describe("Testing for the BaseWidget class basic functions", () => {
    test("Testing that the WidgetParent constructor works for IDs and HTMLElements", () => {
        expect(new WidgetParent("parent")).toStrictEqual(new WidgetParent(document.getElementById("parent") ?? function() { throw new Error("Parent not found")}()))
    })

    test("Testing all the upper layer getters", () => {
        expect(test_widget.get_name()).toBe("test_widget_name");
        expect(test_widget.get_id()).toBe("silly_test");
    })

    test("Testing the empty module", async () => {
        await expect(test_widget.run()).rejects.toThrow(EvalError);
    })

    describe("Testing WidgetRequestModule integration with BaseWidget", () => {
        test("Testing that static connection characteristics are functioning well", () => {
            WidgetRequestModule.set_connection_information("test", 123);
            expect(TestBasicUpdaterRequest.host).toBe("test");
            expect(TestBasicUpdaterRequest.port).toBe(123);
        });

        test("Testing the basic request module", () => {
            expect(test_basic_updater_request.generate_request_body()).toStrictEqual({method:"GET"});
        });
    
        test("Testing integration of basic request module with widget", async () => {
            test_widget.add_core_module(test_basic_updater_request);
            await test_widget.run();
    
            expect(test_widget.get_value()).toStrictEqual({"test_field1":"silly1", "test_field2":"silly2"});
            expect(document.getElementById("test_field2")?.innerHTML).toBe("silly2");
        })
    });

    describe("Testing integration of WidgetListenerModule with BaseWidget", () => {
        let test_button_widget: BaseWidget;
        let listener_module: WidgetListenerModule;
        let test_html_controller: TestButtonHTMLController;
        let test_function: (controller: AbstractBaseWidgetHTMLController<BaseStartupFieldParameters>) => Promise<void>;
        let widget_parent =  new WidgetParent("parent");

        beforeAll(() => {
            test_html_controller = new TestButtonHTMLController(
                new TestButtonStartupFields("test_button", "test_button")
            );

            test_button_widget = new BaseWidget(
                test_html_controller,
                new BaseWidgetMetadata("test_button", widget_parent)
            );
            listener_module = new WidgetListenerModule("click")
                .inject_dependency(test_widget);

            test_function = async (controller: AbstractBaseWidgetHTMLController<BaseStartupFieldParameters>) => {
                controller.update_dynamic_fields({"test_button":"silly_hehe"})
            }

            listener_module.inject_operation(test_function);

            test_button_widget.add_listener_module(listener_module);
        })
    
        test("Testing to see if the button widget html was configured properly", () => {
            expect(document.getElementById("test_button")?.innerHTML).toBe("test_button");
            expect(document.getElementById("test_button")?.id).toBe("test_button");
        })
    
        test("Testing to see if clicking the button propogates necessary changes throughout the stack", () => {
            document.getElementById("test_button")?.click();
            //listener_module.run_new();
            expect(test_widget.get_value()).toStrictEqual({"test_field1":"silly1", "test_field2":"silly2"});
            expect(document.getElementById("test_field2")?.innerHTML).toBe("silly2");

            expect(test_button_widget.get_value()).toStrictEqual({"test_button":"silly_hehe"})
            expect(document.getElementById("test_button")?.innerHTML).toBe("silly_hehe");
        })
    });

    describe("Testing the function of the repetitive module wrapper", () => {
        let repetitive_module: RepetitiveModuleTest;

        beforeAll(() => {
            repetitive_module = new RepetitiveModuleTest();
            test_widget.add_module_repetitive(repetitive_module, 5);
        });

        test("Testing to see if the module executes 3 times in 15 seconds", async () => {
            await new Promise(r => setTimeout(r, 15));
            expect(repetitive_module.get_run_count()).toBeGreaterThanOrEqual(3);
        })

        test("Testing to see if the repetitive task can be killed", async () => {
            test_widget.kill_module_repetitive();
            await new Promise(r => setTimeout(r, 15));
            expect(repetitive_module.get_run_count()).toBeGreaterThanOrEqual(3);
        })
    })
})

