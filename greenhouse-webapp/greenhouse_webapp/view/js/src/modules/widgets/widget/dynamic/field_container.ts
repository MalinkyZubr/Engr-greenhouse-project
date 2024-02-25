import { DynamicField, StaticField } from "./fields";
import { HTMLControllerError } from "../../../exceptions/module_errors";

export type FieldParameters = {
    [key: string]: string
}

export type ListParameters = {
    [key: string]: FieldParameters
}

export abstract class AbstractFieldContainer<FieldType extends DynamicField> {
    private fields: Map<string, FieldType>;
    private widget_node: HTMLElement; 

    constructor(widget_node: HTMLElement, data_attribute_name: string) { // receive the BaseFieldContainerParameters here and create all necessary fields with the factory method
        this.widget_node = widget_node;
        this.fields = this.locate_fields(data_attribute_name);
    }

    public equals(parameters: FieldParameters): boolean {
        for(const [field_name, field] of Object.entries(parameters)) {
            var container_entry: FieldType | undefined = this.fields.get(field_name);

            if(!container_entry) {
                throw new HTMLControllerError("FIELDS_ERROR", 
                    `Container does not contain field ${field_name}`, this);
            }
            else if(field != container_entry.get_value()) {
                return false;
            }
        }
        return true;
    }

    private locate_fields(data_attribute_name: string): Map<string, FieldType> {
        var logical_fields: Map<string, FieldType> = new Map<string, FieldType>();

        var html_fields: NodeListOf<HTMLElement> = this.widget_node.querySelectorAll(`[data-field="${data_attribute_name}"]`);
        var tag_data_field: string | null = this.widget_node.getAttribute("data-field");

        if(tag_data_field && html_fields.length > 0) {
            throw new HTMLControllerError("HTML_TEMPLATE_ERROR", 
                "The widget has runtime fields (dynamic/static) in both in the root tag and as children. Widgets may not have both", this)
        }
        else if(tag_data_field) {
            logical_fields.set(this.widget_node.id, this.create_field(this.widget_node))
        }
        else {
            for(var index = 0; index < html_fields.length; index++) {
                const field: HTMLElement = html_fields[index];
                var field_id: string = field.id;
                var field_object: FieldType = this.create_field(field);
                logical_fields.set(field_id, field_object);
            }
        }

        return logical_fields;
    }

    public set_field_values(field_data: FieldParameters): void {
        for(const [key, field] of this.fields.entries()) {
            if(field_data.hasOwnProperty(key)) {
                var value_to_set: string = field_data[key];
                field.set_value(value_to_set);
            }
        }
    }

    public get_field_values(): FieldParameters {
        var values_map: FieldParameters = {};
        for(const [key, field] of this.fields.entries()) {
            values_map[key] = field.get_value();
        }
        return values_map;
    }

    abstract create_field(field_node: HTMLElement): FieldType;
}

export class DynamicFields extends AbstractFieldContainer<DynamicField> {
    public constructor(widget_node: HTMLElement) {
        super(widget_node, "dynamic");
    }

    public create_field(field_node: HTMLElement): DynamicField {
        return new DynamicField(field_node);
    }
}

export class StaticFields extends AbstractFieldContainer<StaticField> {
    public constructor(widget_node: HTMLElement) {
        super(widget_node, "static");
    }

    public create_field(field_node: HTMLElement): StaticField {
        return new StaticField(field_node);
    }
}