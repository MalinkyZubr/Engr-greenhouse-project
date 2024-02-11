import { WidgetParentData, Widget } from "../widget";
import { ListElement } from "../list/elementList";


export class RadioButton extends ListElement {
    widget_name: string = "RadioButton"
    widget_html: string = 
    `<input type="radio" id={{ element_id }} name={{ radio_group_name }} value={{ button_value }}>
    <label for={{ element_id }}>{{ button_label }}</label><br>`

    constructor(widget_data: object, parent_data: WidgetParentData) {
        super(widget_data, parent_data);
    }

    public is_checked(): boolean {
        return (this.get_widget_node() as HTMLInputElement).checked;
    }

    public get_value(): string {
        return (this.get_widget_node() as HTMLInputElement).value;
    }
}

export abstract class RadioGroup extends Widget {
    abstract widget_name: string;
    widget_html: string = 
    `<div id={{ element_id }}>
        {{ radio_button_group }}
    </div>`

    private radio_button_group: Array<RadioButton>;
    private radio_button_group_name: string;
    private radio_button_group_value: string;

    constructor(widget_data: object, parent_element: WidgetParentData) {
        super(widget_data, parent_element);

        this.radio_button_group = Array<RadioButton>();
        this.radio_button_group_name = widget_data["radio_button_group"];

        this.get_widget_node().addEventListener("change", this.radio_select_event);
    }

    private radio_select_event() {
        for(var x = 0; x < this.radio_button_group.length; x++) {
            var radio_button: RadioButton = this.radio_button_group[x];
            if(radio_button.is_checked()) {
                this.radio_button_group_value = radio_button.get_value();
            }
        }
    }

    public add(button_label: string): RadioGroup {
        var new_button = new RadioButton(
            {
                element_id: button_label,
                radio_group_name: this.radio_button_group_name,
                button_value: button_label,
                button_label: button_label
            },
            new WidgetParentData(this.get_id())
        )

        this.radio_button_group.push(new_button);

        return this;
    }

    public get_value(): string {
        return this.radio_button_group_value;
    }
}