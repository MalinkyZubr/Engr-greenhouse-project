import { host, port } from "../config.ts"

const find_template_params: string = "(?<=\\{{ )(.*?)(?=\\ }})";
const find_template_id_fields: string = "(?<=id=)['\"]?([^'\"\\s]+)['\"]?"
const extract_from_quotes: string = "(?<=')[^']+?(?=')"


class BasePage {
    private nodes: Array<HTMLElement>;
    private widgets: Map<string, AbstractBaseWidget<BaseStartupFieldParameters, BaseFieldContainerParameters, BaseFieldContainerParameters>>;

    public get_widgets(): Map<string, AbstractBaseWidget<BaseStartupFieldParameters, BaseFieldContainerParameters, BaseFieldContainerParameters>> {
        return this.widgets;
    }

    public add_widget(widget: AbstractBaseWidget<BaseStartupFieldParameters, BaseFieldContainerParameters, BaseFieldContainerParameters>, parent_id: string): BasePage {

        return this;
    }
}

class BaseWidgetMetadata {
    private widget_name: string;
    private widget_parent_element: WidgetParentReference;
    private widget_parent_page: BasePage;

    constructor(widget_name: string, widget_parent: WidgetParentReference, widget_parent_page: BasePage) {
        this.widget_name = widget_name;
        this.widget_parent_element = widget_parent;
        this.widget_parent_page = widget_parent_page;
    }

    public get_widget_name(): string {
        return this.widget_name;
    }

    public get_page(): BasePage {
        return this.widget_parent_page;
    }

    public get_widget_parent_element(): WidgetParentReference {
        return this.widget_parent_element;
    }
}

abstract class BaseFieldContainerParameters {
    public get_key_values_map(): object {
        const attributes: Array<string> = Object.getOwnPropertyNames(this).filter(field => typeof(this[field]) != "function");
        var properties: object = {};

        for(const attribute in attributes) {
            const descriptor = Object.getOwnPropertyDescriptor(this, attribute) ?? function() { throw new Error(`Field ${attribute} not found`)}();
            const value = descriptor.value;

            properties[attribute] = value;
        }

        return properties;
    }

    public get_value(key: string) {
        const key_value_pairs = this.get_key_values_map();

        return key_value_pairs[key];
    }

    public get_values(): string[] {
        const key_value_pairs = this.get_key_values_map();

        return Object.values(key_value_pairs);
    }

    public get_keys(): string[] {
        const key_value_pairs = this.get_key_values_map();

        return Object.keys(key_value_pairs);
    }

    public set_parameters_object(parameters_object: object): void {
        for(const [key, value] of Object.entries(parameters_object)) {
            if(!(key in Object.getOwnPropertyNames(this))) {
                throw new Error(`Key ${key} not found in field parameters`);
            }
            this[key] = value;
        }
    }

    abstract set_parameters(...args: any[]): void;
}

class DynamicField {
    private value: string;
    private field: HTMLElement;

    public constructor(field: HTMLElement) {
        this.field = field;
    }

    public get_value(): string {
        return this.value;
    }

    public set_value(value: string): void {
        this.value = value;
        this.field.innerText = value;
    }
}

class StaticField extends DynamicField {
    private is_set: boolean = false;

    public set_value(value: string): void {
        if(!this.is_set) {
            super.set_value(value);
        }
        else {
            throw new Error(`static, can only be set once!`);
        }
    }
}

abstract class AbstractBaseFieldContainer<FieldType extends DynamicField, FieldParameters extends BaseFieldContainerParameters> {
    private fields: Map<string, FieldType>;
    abstract data_attribute_name: string;
    private widget_node: HTMLElement; 

    constructor(widget_node: HTMLElement) { // receive the BaseFieldContainerParameters here and create all necessary fields with the factory method
        this.widget_node = widget_node;
        this.fields = this.locate_fields();
    }

    private locate_fields(): Map<string, FieldType> {
        var html_fields: NodeListOf<HTMLElement> = this.widget_node.querySelectorAll(`[data-field-type='${this.data_attribute_name}']`);
        var logical_fields: Map<string, FieldType> = new Map<string, FieldType>();

        for(var index = 0; index < html_fields.length; index++) {
            const field: HTMLElement = html_fields[index];
            var field_id: string = field.id;
            var field_object: FieldType = this.create_field(field);
            logical_fields.set(field_id, field_object);
        }

        return logical_fields;
    }

    public set_field_values(field_data: FieldParameters): void {
        for(var index = 0; index < this.fields.size; index++) {
            var field: FieldType = this.fields.values()[index];
            var field_id: string = this.fields.keys()[index];
            field.set_value(field_data[field_id]);
        }
    }

    abstract create_field(field_node: HTMLElement): FieldType;
}

class DynamicFields<FieldParameters extends BaseFieldContainerParameters> extends AbstractBaseFieldContainer<DynamicField, FieldParameters> {
    public data_attribute_name: "DYNAMIC";

    public create_field(field_node: HTMLElement): DynamicField {
        return new DynamicField(field_node);
    }
}

class StaticFields<FieldParameters extends BaseFieldContainerParameters> extends AbstractBaseFieldContainer<StaticField, FieldParameters> {
    public data_attribute_name: string = "STATIC";

    public create_field(field_node: HTMLElement): StaticField {
        return new StaticField(field_node);
    }
}

class BaseStartupFieldParameters extends BaseFieldContainerParameters {
    private element_id: string;

    public get_element_id(): string {
        return this.element_id;
    }

    public set_parameters(element_id: string): void {
        this.element_id = element_id
    }
}


abstract class AbstractBaseWidgetHTMLController<StartupFieldsDataType extends BaseStartupFieldParameters, DynamicFieldsDataType extends BaseFieldContainerParameters, StaticFieldsDataType extends BaseFieldContainerParameters> {
    abstract widget_html: string;
    abstract update_route: string;
    private widget_node: HTMLElement;
    private static_fields: StaticFields<StaticFieldsDataType>;
    private dynamic_fields: DynamicFields<DynamicFieldsDataType>;
    private startup_fields: StartupFieldsDataType;
    // private parent_page: BasePage; replace with dependency injection

    constructor(widget_node: HTMLElement, startup_field_data: StartupFieldsDataType) {
        this.check_for_id_field(startup_field_data);

        this.widget_node = widget_node
        this.dynamic_fields = new DynamicFields(widget_node);
        this.static_fields = new StaticFields(widget_node);

        this.set_startup_fields(startup_field_data, this.get_widget_html_string());
        this.startup_fields = startup_field_data;
    }

    private get_widget_html_string(): string {
        return this.widget_html;
    }

    private check_for_id_field(startup_field_data: StartupFieldsDataType): void {
        if(!this.widget_html.includes(`id={{ element_id }}`)) {
            throw new Error(`The html for the widget id=${startup_field_data.get_element_id()} does not have an element_id field`)
        }
    }

    private set_startup_fields(startup_field_data: StartupFieldsDataType, html_template: string): string {
        for(const [key, value] of Object.entries(startup_field_data.get_key_values_map())) {
            html_template = this.set_startup_field(key, value, html_template);
        }
        return html_template;
    }

    private set_startup_field(key: string, value: string, html_template: string): string { // these should go in a separate class
        var reformatted_html: string = html_template.replace(`{{ ${key} }}`, value);
        if(reformatted_html === html_template) {
            throw new Error(`Startup field ${key} not found in ${this.startup_fields.get_element_id()}`);
        }
        return reformatted_html;
    }

    public get_id() {
        return this.startup_fields.get_element_id();
    }

    public async get_update(): Promise<void> {
        const request_path = `https://${host}:${port}${this.update_route}`
        var dynamic_update_parameters: DynamicFieldsDataType = this.generate_dynamic_field_parameters()

        await fetch(request_path, {
            method: "GET"
        })
        .then(res => res.json())
        .then(res => {
            dynamic_update_parameters.set_parameters_object(res);
        });

        dynamic_update_parameters = await this.update_processing(dynamic_update_parameters);

        this.dynamic_fields.set_field_values(dynamic_update_parameters);
    }

    abstract generate_dynamic_field_parameters(): DynamicFieldsDataType;

    abstract update_processing(field_data: DynamicFieldsDataType): Promise<DynamicFieldsDataType>;
}



class WidgetParentReference {
    element_id: string;
    element: HTMLElement;

    constructor(element: string | HTMLElement) {
        if(typeof(element) == "string") {
            this.element_id = element;
            this.element = document.getElementById(element) ?? function() { throw new Error(`Element ${element} not found`)}();
        }
        else {
            element = element as HTMLElement;
            this.element_id = element.id;
            this.element = element;
        }
    }

    public get_id(): string {
        return this.element_id;
    }

    public static by_id(id: string): WidgetParentReference {
        return new WidgetParentReference(id);
    }

    public static by_element(element: HTMLElement): WidgetParentReference {
        return new WidgetParentReference(element);
    }
}

abstract class AbstractBaseWidget<StartupFieldsDataType extends BaseStartupFieldParameters, DynamicFieldsDataType extends BaseFieldContainerParameters, StaticFieldsDataType extends BaseFieldContainerParameters> {
    abstract widget_name: string;
    private widget_html: AbstractBaseWidgetHTMLController<StartupFieldsDataType, DynamicFieldsDataType, StaticFieldsDataType>;
    private widget_metadata: BaseWidgetMetadata; 

    private generate_formatted_html(widget_data: object) {}

    public get_name(): string {
        return this.widget_metadata.get_widget_name();
    }

    public get_id(): string {
        return this.widget_html.get_id();
    }

    public get_widget_name(): string {
        return this.widget_name;
    }

    public async run() {
        this.widget_html.get_update();

    }
}