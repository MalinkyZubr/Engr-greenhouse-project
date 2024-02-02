import { DynamicField, StaticField } from "./fields";

export abstract class AbstractFieldContainer<FieldType extends DynamicField> {
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

    public set_field_values(field_data: object): void {
        for(var index = 0; index < this.fields.size; index++) {
            var field: FieldType = this.fields.values()[index];
            var field_id: string = this.fields.keys()[index];
            field.set_value(field_data[field_id]);
        }
    }

    abstract create_field(field_node: HTMLElement): FieldType;
}

export class DynamicFields extends AbstractFieldContainer<DynamicField> {
    public data_attribute_name: "DYNAMIC";

    public create_field(field_node: HTMLElement): DynamicField {
        return new DynamicField(field_node);
    }
}

export class StaticFields extends AbstractFieldContainer<StaticField> {
    public data_attribute_name: string = "STATIC";

    public create_field(field_node: HTMLElement): StaticField {
        return new StaticField(field_node);
    }
}