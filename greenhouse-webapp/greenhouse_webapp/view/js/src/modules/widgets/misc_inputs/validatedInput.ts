import { WidgetListenerModule } from "../widget/widget";
import { AbstractBaseWidgetHTMLController, BaseStartupFieldParameters } from "../widget/dynamic/widget_html";
import { FieldParameters } from "../widget/dynamic/field_container";


export class ValidatedInputStartupParameters extends BaseStartupFieldParameters {
    private placeholder: string;
    private error_message: string;

    constructor(element_id: string, placeholder: string, error_message: string) {
        super(element_id);
        this.placeholder = placeholder;
        this.error_message = error_message;
    }
}


export class ValidatedInputHTMLController extends AbstractBaseWidgetHTMLController<ValidatedInputStartupParameters> {
    public html_template_generator(): string {
        return `<div id={{ element_id }}>
                    <input type="text" placeholder="{{ placeholder }}" id="input">
                    <br>
                    <p id="error" class="input-error">{{ error_message }}</p>
                </div>`
    }

    private value: string = "";
    private compliant: boolean = false;
    private input_field: HTMLInputElement | null = null;
    private validation_regex: RegExp

    constructor(startup_field_parameters: ValidatedInputStartupParameters, validation_regex: string) {
        super(startup_field_parameters);
        this.validation_regex = new RegExp(validation_regex);
    }

    public assign_widget_node(widget_node: HTMLElement): AbstractBaseWidgetHTMLController<ValidatedInputStartupParameters> {
        super.assign_widget_node(widget_node);

        var input_field: HTMLInputElement | null = this.get_node().querySelector("#input");

        if(!input_field) {
            throw new Error(`${this.get_startup_fields().get_element_id()} has no input`);
        }
        else {
            this.input_field = input_field;
        }

        return this;
    }

    public get_value(): FieldParameters {
        if(!this.compliant) {
            throw new Error(`${this.get_id()} input failed validation. ${this.value} does not follow regex pattern assigned`);
        }

        var output: FieldParameters = {}
        output[this.get_id()] = this.value;

        return output
    }

    private extract_value_from_html(): string {
        if(!this.input_field) {
            throw new Error(`${this.constructor.name} was not attached to any BaseWidget object, and does not have a DOM node`);
        }
        return this.input_field.value;
    }

    private toggle_error(compliant: boolean) {
        var error_field: HTMLElement | null = this.get_node().querySelector("#error") 
        if(!error_field) {
            throw new Error(`Error field does not exist for ${this.constructor.name}`);
        }

        if(!compliant) {
            error_field.style.display = "none";
        }
        else {
            error_field.style.display = "block";
        }
    }

    public validate_input() {
        var input_value: string = this.extract_value_from_html();
        var is_compliant: boolean = this.validation_regex.test(input_value);

        this.compliant = is_compliant;

        if(is_compliant) {
            this.value = input_value;
        }

        this.toggle_error(is_compliant);
    }
}