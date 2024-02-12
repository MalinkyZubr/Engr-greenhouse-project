import { DynamicFields, StaticFields } from "./field_container";
import type { FieldParameters } from "./field_container"


export class BaseStartupFieldParameters {
    private element_id: string;

    constructor(element_id: string) {
        this.element_id = element_id;
    }

    public get_element_id(): string {
        return this.element_id;
    }

    private set_startup_fields(html_template: string): string {
        for (const key of Object.getOwnPropertyNames(this)) {
            var object_key: string = key;
            var object_value: string | null = Object.getOwnPropertyDescriptor(this, object_key)?.value;

            // if (!object_value && object_key != "0") {
            //     console.warn(`Field ${object_key} does not exist for ${this.get_element_id()}`)
            // }
            if(object_value && object_key != "0") {
                html_template = this.set_startup_field(key, object_value, html_template)
            }
        }
        return html_template;
    }

    private set_startup_field(key: string, value: string, html_template: string): string { // these should go in a separate class
        var reformatted_html: string = html_template.replace(`{{ ${key} }}`, value);
        if (reformatted_html === html_template) {
            throw new Error(`Startup field ${key} not found in ${this.get_element_id()}`);
        }
        return reformatted_html;
    }

    public assign_to_widget_html(html_template: string): string {
        return this.set_startup_fields(html_template);
    }
}

export abstract class AbstractBaseWidgetHTMLController<StartupFieldsDataType extends BaseStartupFieldParameters> {
    private widget_html_template: string = "";
    private widget_node: HTMLElement | null = null;
    private static_fields: StaticFields | null = null;
    private dynamic_fields: DynamicFields | null = null;
    private startup_fields: StartupFieldsDataType;
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
            throw new Error(`The html for the widget id=${startup_field_data.get_element_id()} does not have an element_id field`)
        }
    }

    public get_id() {
        return this.startup_fields.get_element_id();
    }

    public assign_widget_node(widget_node: HTMLElement): AbstractBaseWidgetHTMLController<StartupFieldsDataType> {
        this.widget_node = widget_node;

        this.widget_node.innerHTML = this.widget_html_template;

        this.dynamic_fields = new DynamicFields(this.widget_node);
        this.static_fields = new StaticFields(this.widget_node);

        return this;
    }

    protected get_node(): HTMLElement {
        return this.widget_node ?? function () { throw new Error("Node is not yet defined, be sure to run assign_widget_node!") }();
    }

    public update_dynamic_fields(field_data: FieldParameters) {
        this.dynamic_fields?.set_field_values(field_data);
    }

    private get_startup_fields(): StartupFieldsDataType {
        return this.startup_fields;
    }

    public extract_child(id: string): HTMLElement {
        return this.widget_node?.querySelector(`#${id}`) ?? function () { throw new Error(`Child of ID ${id} does not exist on HTMLElement`) }();
    }

    protected get_static_fields(): StaticFields | null {
        return this.static_fields;
    }

    protected get_dynamic_fields(): DynamicFields | null {
        return this.dynamic_fields;
    }

    public get_value(): FieldParameters {
        return (this.get_dynamic_fields() ?? function() { throw new Error("Field parameters do not exist") }()).get_field_values();
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