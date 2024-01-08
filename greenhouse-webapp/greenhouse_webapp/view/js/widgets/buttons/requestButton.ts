import { Widget, WidgetParentData } from "../widget";


export abstract class RequestButton extends Widget {
    widget_name: string = "RequestButton";
    widget_html: string = 
    `<button class="button" id="{{ element_id }}">{{ button_name }}</button>`

    constructor(widget_data: object, parent_element: WidgetParentData) {
        super(widget_data, parent_element);

        this.get_widget_node().addEventListener("click", this.submit_button_request)
    }

    abstract submit_button_request(): Promise<void>;
}