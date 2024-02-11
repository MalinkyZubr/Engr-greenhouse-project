import { DynamicField, StaticField } from "./fields";

export type FieldParameters = {
    [key: string]: string
}

export abstract class AbstractFieldContainer<FieldType extends DynamicField> {
    private fields: Map<string, FieldType>;
    private widget_node: HTMLElement; 

    constructor(widget_node: HTMLElement, data_attribute_name: string) { // receive the BaseFieldContainerParameters here and create all necessary fields with the factory method
        this.widget_node = widget_node;
        this.fields = this.locate_fields(data_attribute_name);
    }

    private locate_fields(data_attribute_name: string): Map<string, FieldType> {
        var html_fields: NodeListOf<HTMLElement> = this.widget_node.querySelectorAll(`[data-field="${data_attribute_name}"]`);

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