import { HTMLControllerError, WidgetError } from "../../../exceptions/module_errors";
import { DynamicFields, StaticFields } from "./field_container";
import type { FieldParameters, ListParameters } from "./field_container"


export type WrapperTypes = "div" | "tr" | "td" 


export class BaseStartupFieldParameters {
    private element_id: string;

    constructor(element_id: string) {
        this.element_id = element_id;
    }

    public get_element_id(): string {
        return this.element_id;
    }

    private generate_tooltip(tooltip_data: FieldParameters): string {
        var tooltip_string: string = "";
        for(const [key, value] of Object.entries(tooltip_data)) {
            tooltip_string = tooltip_string.concat(`${key}: ${value}\n`);
        }
        return tooltip_string;
    }

    private set_startup_fields(html_template: string): string {
        for (const key of Object.getOwnPropertyNames(this)) {
            var object_key: string = key;
            var object_value: string | FieldParameters | null = Object.getOwnPropertyDescriptor(this, object_key)?.value;

            if(object_value && object_value != "0") {
                if(typeof(object_value) != "string") {
                    object_value = this.generate_tooltip(object_value);
                }
                html_template = this.set_startup_field(key, object_value, html_template);
            }
        }
        return html_template;
    }

    private set_startup_field(key: string, value: string, html_template: string): string { // these should go in a separate class
        var reformatted_html: string = html_template.replace(`{{ ${key} }}`, value);
        if (reformatted_html === html_template) {
            throw new HTMLControllerError("FIELDS_ERROR", 
                `Startup field ${key} not found`, this);
        }
        return reformatted_html;
    }

    public assign_to_widget_html(html_template: string): string {
        return this.set_startup_fields(html_template);
    }
}


export interface HTMLController{
    get_id(): string;
    get_node(): HTMLElement;
    extract_child(id: string): HTMLElement
    equals(parameters: FieldParameters): boolean;
    update_dynamic_fields(field_data: FieldParameters): void;
    get_value(): FieldParameters | ListParameters;
}


export abstract class AbstractBaseWidgetHTMLController<StartupFieldsDataType extends BaseStartupFieldParameters> implements HTMLController {
    private widget_html_template: string = "";
    protected widget_node: HTMLElement | null = null;
    private static_fields: StaticFields | null = null;
    private dynamic_fields: DynamicFields | null = null;
    private startup_fields: StartupFieldsDataType;
    private to_execute_with_dynamic_update: null | ((controller: AbstractBaseWidgetHTMLController<StartupFieldsDataType>, dynamic_fields: FieldParameters) => void) = null;

    // private parent_page: BasePage; replace with dependency injection

    constructor(startup_field_data: StartupFieldsDataType) {
        this.check_for_id_field(startup_field_data);
        this.startup_fields = startup_field_data;
        this.set_widget_html_template(this.startup_fields.assign_to_widget_html(this.html_template_generator()));
    }

    private set_widget_html_template(template: string) {
        this.widget_html_template = template;
    }

    private check_for_id_field(startup_field_data: StartupFieldsDataType): void {
        if (!this.html_template_generator().includes(`id={{ element_id }}`)) {
            throw new HTMLControllerError("HTML_TEMPLATE_ERROR", 
            "The html template for the widget does not have an {{ element_id }} field", this)
        }
    }

    public get_id(): string {
        return this.startup_fields.get_element_id();
    }

    public assign_widget_node(widget_node: HTMLElement): AbstractBaseWidgetHTMLController<StartupFieldsDataType> {
        widget_node.innerHTML = this.widget_html_template;
        var actual_node: HTMLElement | null = widget_node.querySelector(`#${this.get_id()}`);

        if(!actual_node) {
            throw new WidgetError("IMPROPER_CONFIGURATION_ERROR", 
                "Critical error assigning widget node to html controller, id not found in node", this)
        }

        this.widget_node = actual_node;

        this.dynamic_fields = new DynamicFields(this.widget_node);
        this.static_fields = new StaticFields(this.widget_node);

        return this;
    }

    public get_node(): HTMLElement {
        var selected_node: HTMLElement | null = this.widget_node;
        
        if(!selected_node) {
            throw new WidgetError("NONEXISTANT_CONFIGURATION_ERROR", 
                "Node is not yet defined, be sure to run assign_widget_node!", this)
        }

        return selected_node;
    }

    public equals(parameters: FieldParameters): boolean {
        if(!this.dynamic_fields) {
            throw new HTMLControllerError("FIELDS_ERROR", 
                "No dynamic fields on HTML controller", this)
        }
        return this.dynamic_fields.equals(parameters);
    }

    public execute_with_dynamic_update(to_execute: (controller: AbstractBaseWidgetHTMLController<StartupFieldsDataType>, parameters: FieldParameters) => void): AbstractBaseWidgetHTMLController<StartupFieldsDataType> {
        this.to_execute_with_dynamic_update = to_execute;
        return this;
    }

    public update_dynamic_fields(field_data: FieldParameters): void {
        if(!this.dynamic_fields) {
            throw new HTMLControllerError("FIELDS_ERROR", 
                "No dynamic fields on HTML controller", this)
        }
        this.dynamic_fields.set_field_values(field_data);
        if(this.to_execute_with_dynamic_update) {
            this.to_execute_with_dynamic_update(this, field_data);
        }
    }

    public get_startup_fields(): StartupFieldsDataType {
        return this.startup_fields;
    }

    public extract_child(id: string): HTMLElement {
        var extracted_child_node: HTMLElement | null | undefined = this.widget_node?.querySelector(`#${id}`);

        if(!extracted_child_node) {
            throw new WidgetError("CHILD_ERROR", 
                `Child node with id ${id} does not exist in HTML controller`, this)
        }

        return extracted_child_node
    }

    protected get_static_fields(): StaticFields | null {
        return this.static_fields;
    }

    protected get_dynamic_fields(): DynamicFields | null {
        return this.dynamic_fields;
    }

    public get_value(): FieldParameters | ListParameters {
        return (this.get_dynamic_fields() ?? function() { throw new Error("Field parameters do not exist") }()).get_field_values();
    }

    public wrap_node(wrapper_type: WrapperTypes): AbstractBaseWidgetHTMLController<StartupFieldsDataType> {
        var wrapper_node: HTMLTableRowElement | HTMLDivElement | HTMLTableCellElement = document.createElement(wrapper_type);
        wrapper_node.appendChild(this.get_node());
        this.widget_node = wrapper_node;

        return this;
    }

    /**
     * This function should return the html template to be used by the widget
     * 
     * The data-field= tag should be used to specify if a field is "dynamic" or "static"
     * 
     * Dynamic Fields: can be dynamically updated by the widget at any point during runtime
     * 
     * Static Fields: can only be updated once at the start of runtime (making them similar to startup fields)
     */
    abstract html_template_generator(): string;
}