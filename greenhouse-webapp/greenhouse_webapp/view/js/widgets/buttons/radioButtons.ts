import { StandardWidget } from "../widget";


export abstract class RadioButton extends StandardWidget {
    abstract widget_name: string;
    widget_html: string = 
    `<input type="radio" id={{ element_id }} name={{ radio_group_name }} value={{ button_value }}>
    <label for={{ element_id }}>Temperature (c)</label><br>`

    private radio_button_group: NodeListOf<HTMLInputElement>;
    public static radio_button_group_value: string;

    constructor(widget_data: object, parent_element: HTMLElement, widget_key: string) {
        super(widget_data, parent_element, widget_key);

        this.radio_button_group = parent_element.querySelectorAll(`[name='${widget_data["radio_group_name"]}']`);
        this.get_widget_node().addEventListener("change", this.radio_select_event);
    }

    abstract set_static_value(value: string): void;

    private radio_select_event() {
        for(var x = 0; x < this.radio_button_group.length; x++) {
            var radio_button: HTMLInputElement = this.radio_button_group[x];
            if(radio_button.checked) {
                this.set_static_value(radio_button.value);
            }
        }
    }
}