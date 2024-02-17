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
    private input_field: HTMLInputElement;

    constructor(startup_field_data: ValidatedInputStartupParameters) {
        super(startup_field_data);

        var input_field: HTMLInputElement | null = this.get_node().querySelector("#input");

        if(!input_field) {
            throw new Error(`${this.get_startup_fields().get_element_id()} has no input`);
        }
        else {
            this.input_field = input_field;
        }
    }

    public get_value(): FieldParameters {
        if(!this.compliant) {
            throw new Error(`${this.get_id()} input failed validation`);
        }

        var output: FieldParameters = {}
        output[this.get_id()] = this.value;

        return output
    }

    private extract_value_from_html(): string {
        return this.input_field.value;
    }

    private toggle_error(error: boolean) {
        var error_field: HTMLElement | null = this.get_node().querySelector("#error") 
        if(!error_field) {
            throw new Error(`Error field does not exist for ${this.constructor.name}`);
        }

        if(error) {
            error_field.style.display = "none";
        }
        else {
            error_field.style.display = "block";
        }
    }

    public validate_input(validation_regex: RegExp) {
        var input_value: string = this.extract_value_from_html();
        var is_compliant: boolean = validation_regex.test(input_value);

        this.compliant = is_compliant;

        if(is_compliant) {
            this.value = input_value;
        }

        this.toggle_error(is_compliant);
    }
}


export class ValidatedInputModule extends WidgetListenerModule<ValidatedInputHTMLController> {
    private validation_regex: RegExp;

    constructor(listen_event: string, validation_regex: string, listening_element_id: string) {
        super(listen_event, listening_element_id);
        this.validation_regex = new RegExp(validation_regex);
    }

    public validate_input() {
        this.get_widget_html_controller().validate_input(this.validation_regex);
    }
}