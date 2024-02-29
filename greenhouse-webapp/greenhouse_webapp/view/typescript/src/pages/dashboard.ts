import { Page } from "../modules/page/page";
import { WidgetParent, BaseWidget, BaseWidgetMetadata, WidgetRequestModule, Requester } from "../modules/widgets/widget/widget";
import { TableHTMLController } from "../modules/widgets/list_widgets/elementTable";
import { PushButton } from "../modules/widgets/buttons/pushButton";
import { AbstractBaseWidgetHTMLController, BaseStartupFieldParameters } from "../modules/widgets/widget/dynamic/widget_html";
import { ListAppendModule, WidgetListHTMLController } from "../modules/widgets/widget/list_widget";
import { port, host } from "./config";
import { FieldParameters } from "../modules/widgets/widget/dynamic/field_container";


let dashboard_page = new Page();


Requester.set_connection_information(host, port);


class DevicesChildWidgetController extends PushButton {
    public html_template_generator(): string {
        return `<div id="{{ element_id }}>
            <button class="button" id="id" data-field="dynamic"></button>
            <button class="button" id="state" data-field="dynamic"></button>
        </div>`
    }
} 


class TableAppendModule extends ListAppendModule<PushButton> {
    public generate_html(element_id: string, elements_field: FieldParameters): PushButton {
        return new PushButton(new BaseStartupFieldParameters(element_id));
    }

    public generate_request_body(): object {
        return {method: "GET"}
    }
}


class DeviceTableAppendModule extends ListAppendModule<DevicesChildWidgetController> {
    public generate_html(element_id: string, elements_field: FieldParameters): DevicesChildWidgetController {
        return new DevicesChildWidgetController(new BaseStartupFieldParameters(element_id))
            .execute_with_dynamic_update((controller: DevicesChildWidgetController, dynamic_parameters: FieldParameters) => {
                switch(dynamic_parameters["state"]) {
                    case "ACTIVE":
                        controller.get_node().style.color = "green"
                        break;
                    case "IDLE":
                        controller.get_node().style.color = "yellow"
                        break;
                    case "DISCONNECTED":
                        controller.get_node().style.color = "red";
                        break;
                }
            });
    }

    public generate_request_body(): object {
        return {method: "GET"}
    }
}


let presets_table: BaseWidget = new BaseWidget(
    new TableHTMLController(
        new BaseStartupFieldParameters("presets_table_content"),
    ),
    new BaseWidgetMetadata("presets", 
        new WidgetParent(dashboard_page.consume_element("presets_table")))
).add_module_repetitive(new TableAppendModule("/presets/list_presets", "tr"), 10);


let projects_table: BaseWidget = new BaseWidget(
    new TableHTMLController(
        new BaseStartupFieldParameters("projects_table")
    ),
    new BaseWidgetMetadata("projects",
        new WidgetParent(dashboard_page.consume_element("projects_table")))
).add_module_repetitive(new TableAppendModule("/projects/list_projects", "tr"), 10);


let devices_table: BaseWidget = new BaseWidget(
    new TableHTMLController(
        new BaseStartupFieldParameters("devices_table")
    ),
    new BaseWidgetMetadata("devices",
        new WidgetParent(dashboard_page.consume_element("devices_table")))
).add_module_repetitive(new DeviceTableAppendModule("/devices/list_devices", "tr"), 10);