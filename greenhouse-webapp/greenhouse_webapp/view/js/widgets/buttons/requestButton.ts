import { StandardWidget } from "../widget";


export abstract class RequestButton extends StandardWidget {
    widget_name: string = "RequestButton";
    widget_html: string = 
    `<button class="button" id="{{ element_id }}">{{ button_name }}</button>`

    constructor(widget_data: object, parent_element: HTMLElement, widget_key?: string | undefined) {
        super(widget_data, parent_element, widget_key);

        this.get_widget_node().addEventListener("click", this.submit_button_request)
    }

    abstract submit_button_request(): Promise<void>;
}