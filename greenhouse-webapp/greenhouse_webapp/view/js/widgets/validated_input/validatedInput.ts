import { StandardWidget } from "../widget";


export abstract class ValidatedInput extends StandardWidget {
    widget_name = "ValidatedInput";
    widget_html = 
    `<div id={{ element_id }}>
        <input type="text" placeholder="{{ placeholder }}" id="input">
        <br>
        <p id="error" class="input-error">{{ error_message }}</p>
    </div>`

    abstract format_regex: RegExp;

    constructor(widget_data: object, parent_element: HTMLElement, widget_key?: string | undefined) {
        super(widget_data, parent_element, widget_key);

        var input: Element | null = this.get_widget_node().querySelector("#input")
        if(!input) {
            throw new Error(`input not found for ${this.widget_name}`)
        }

        input.addEventListener("input", this.handle_compliance);
    }

    public get_input_value(): string | null {
        var input_field: HTMLInputElement | null = this.get_widget_node().querySelector("#input");

        if(input_field) {
            return input_field.value;
        }
        return null;
    }

    public check_input_compliance(): boolean {
        var input_value: string | null = this.get_input_value();

        if(input_value) {
            return this.format_regex.test(input_value)
        }
        throw new Error(`Input value not found for ${this.widget_name}`);
    }

    private handle_compliance(): void {
        var compliant: boolean = this.check_input_compliance();
        var error: HTMLElement | null = this.get_widget_node().querySelector("#error") 

        if(!error) {
            throw new Error(`Error field not found for ${this.widget_name}`);
        }

        if(compliant) {
            error.style.display = "none";
        }
        else {
            error.style.display = "block";
        }
    }
}