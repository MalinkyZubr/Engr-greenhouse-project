import { Page } from "../modules/page/page";
import { WidgetParent, BaseWidget, BaseWidgetMetadata, WidgetRequestModule, Requester } from "../modules/widgets/widget/widget";
import { TableHTMLController } from "../modules/widgets/list_widgets/elementTable";
import { PushButton } from "../modules/widgets/buttons/pushButton";
import { AbstractBaseWidgetHTMLController, BaseStartupFieldParameters } from "../modules/widgets/widget/dynamic/widget_html";
import { ListAppendModule, WidgetListHTMLController } from "../modules/widgets/widget/list_widget";
import { port, host } from "./config";
import { FieldParameters } from "../modules/widgets/widget/dynamic/field_container";


let project_page = new Page();


Requester.set_connection_information(host, port);


