import { DynamicFields, StaticFields } from "./field_container";
import { port, host } from "../../../config"
import { WidgetReturn } from "./widget_return";


export class BaseStartupFieldParameters {
    private element_id: string;

    constructor(element_id: string) {
        this.element_id = element_id;
    }

    public get_element_id(): string {
        return this.element_id;
    }

    public set_parameters(element_id: string): void {
        this.element_id = element_id
    }

    private set_startup_fields(html_template: string): string {
        for(const key in Object.getOwnPropertyNames(this)) {
            html_template = this.set_startup_field(key, this[key], html_template);
        }
        return html_template;
    }

    private set_startup_field(key: string, value: string, html_template: string): string { // these should go in a separate class
        var reformatted_html: string = html_template.replace(`{{ ${key} }}`, value);
        if(reformatted_html === html_template) {
            throw new Error(`Startup field ${key} not found in ${this.get_element_id()}`);
        }
        return reformatted_html;
    }

    public assign_to_widget_html(html_template: string): string {
        return this.set_startup_fields(html_template);
    }
}

export abstract class AbstractBaseWidgetHTMLController<StartupFieldsDataType extends BaseStartupFieldParameters> {
    abstract widget_html_template: string;
    private widget_node: HTMLElement;
    private static_fields: StaticFields;
    private dynamic_fields: DynamicFields;
    private startup_fields: StartupFieldsDataType;
    // private parent_page: BasePage; replace with dependency injection

    constructor(startup_field_data: StartupFieldsDataType) {
        this.check_for_id_field(startup_field_data);
        this.startup_fields = startup_field_data;
        this.set_widget_html_template(this.startup_fields.assign_to_widget_html(this.get_widget_html_template()));

        return this;
    }

    private get_widget_html_template(): string {
        return this.widget_html_template;
    }

    private set_widget_html_template(template: string) {
        this.widget_html_template = template;
    }

    private check_for_id_field(startup_field_data: StartupFieldsDataType): void {
        if(!this.widget_html_template.includes(`id={{ element_id }}`)) {
            throw new Error(`The html for the widget id=${startup_field_data.get_element_id()} does not have an element_id field`)
        }
    }

    public get_id() {
        return this.startup_fields.get_element_id();
    }

    public async get_update(): Promise<void> {
        const request_path = `https://${host}:${port}${this.update_route}`
        var dynamic_update_parameters: object = new Object();

        await fetch(request_path, {
            method: "GET"
        })
        .then(res => res.json())
        .then(res => {
            dynamic_update_parameters = res;
        });

        dynamic_update_parameters = await this.update_processing(dynamic_update_parameters);

        this.dynamic_fields.set_field_values(dynamic_update_parameters);
    }

    public assign_widget_node(widget_node: HTMLElement): AbstractBaseWidgetHTMLController<StartupFieldsDataType> {
        this.widget_node = widget_node;

        this.dynamic_fields = new DynamicFields(this.widget_node);
        this.static_fields = new StaticFields(this.widget_node);

        this.widget_node.innerHTML = this.widget_html_template;
        
        return this;
    }

    protected get_node(): HTMLElement {
        return this.widget_node;
    }

    protected get_startup_fields(): StartupFieldsDataType {
        return this.startup_fields;
    }

    abstract update_processing(field_data: object): Promise<object>;

    abstract get_value(): WidgetReturn;
}