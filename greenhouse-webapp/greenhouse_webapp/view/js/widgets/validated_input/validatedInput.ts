import { AbstractBaseWidget } from "../widget/widget.ts";
import { AbstractBaseWidgetHTMLController, BaseStartupFieldParameters } from "../widget/dynamic/widget_html.ts";


export class ValidatedInputStartupParameters extends BaseStartupFieldParameters {
    private placeholder: string;
    private error_message: string;

    constructor(element_id: string, placeholder: string, error_message: string) {
        super(element_id);
        this.placeholder = placeholder;
        this.error_message = error_message;
    }
}

export abstract class ValidatedInput extends AbstractBaseWidgetHTMLController<ValidatedInputStartupParameters> {
    widget_html_template: string = 
    `<div id={{ element_id }}>
        <input type="text" placeholder="{{ placeholder }}" id="input">
        <br>
        <p id="error" class="input-error">{{ error_message }}</p>
    </div>`
    private format_regex: RegExp;

    constructor(startup_field_data: ValidatedInputStartupParameters, validator: RegExp) {
        super(startup_field_data);
        this.format_regex = validator;
    }

    public get_input_value(): string {
        var input_field: HTMLInputElement = this.get_node().querySelector("#input") ?? function() { throw new Error(`${this.get_startup_fields().get_element_id()} has no input`) }();

        if(input_field) {
            return input_field.value;
        }
        return "";
    }

    private check_input_compliance(): boolean {
        var input_value: string | null = this.get_input_value();

        if(input_value) {
            return this.format_regex.test(input_value)
        }
        return true;
    }

    private handle_compliance(): void {
        var compliant: boolean = this.check_input_compliance();
        var error: HTMLElement | null = this.get_node().querySelector("#error") 

        if(!error) {
            throw new Error(`Error field not found for ${this.get_id()}`);
        }

        if(compliant) {
            error.style.display = "none";
        }
        else {
            error.style.display = "block";
        }
    }
}