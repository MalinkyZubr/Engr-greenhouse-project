import { BaseWidget, WidgetModule, WidgetRequestModule, ListParameters, BaseWidgetMetadata, WidgetParent } from "./widget";
import { AbstractBaseWidgetHTMLController } from "./dynamic/widget_html";
import { BaseStartupFieldParameters } from "./dynamic/widget_html"
import { FieldParameters } from "./dynamic/field_container";


enum FieldAction {
    Create,
    Update,
    NoChange
}

export abstract class WidgetListHTMLController<StartupFieldsDataType extends BaseStartupFieldParameters> extends AbstractBaseWidgetHTMLController<StartupFieldsDataType> {
    private children: Map<string, BaseWidget> = new Map<string, BaseWidget>();

    abstract html_template_generator(): string;

    public add_child(child_widget: BaseWidget): void {
        this.children.set(child_widget.get_id(), child_widget);
    }

    public update_child(child_id: string, widget_parameters: FieldParameters):void  {
        this.children.get(child_id)?.set_value(widget_parameters);
    }

    private remove_child(child_id: string): void {
        var child_to_delete: BaseWidget | undefined = this.children.get(child_id);

        if(child_to_delete) {
            child_to_delete.delete_from_dom();
            this.children.delete(child_id);
        }
        else {
            console.warn(`${child_id} not member of list ${this.constructor.name} to delete`);
        }
    }

    public determine_creationary_action(child_id: string, child_parameters: FieldParameters): FieldAction {
        if(!(child_id in this.children.keys())) {
            return FieldAction.Create;
        }
        else {
            if(!this.children.get(child_id)?.equals(child_parameters)) {
                return FieldAction.Update;
            }
            else {
                return FieldAction.NoChange;
            }
        }
    }

    public delete_necessary(requested_children_ids: Array<string>): void {
        const to_delete: Array<string> = Array.from(this.children.keys()).filter(id => {!(id in requested_children_ids)})

        for(const to_delete_id of to_delete) {
            this.remove_child(to_delete_id);
        }
    }
}


export abstract class ListAppendModule<ChildType extends AbstractBaseWidgetHTMLController<BaseStartupFieldParameters>> extends WidgetRequestModule<WidgetListHTMLController<BaseStartupFieldParameters>> {
    abstract generate_html(element_id: string): ChildType;

    abstract generate_request_body(): object;

    public process_response(response: ListParameters): void {
        var widget_html_controller: WidgetListHTMLController<BaseStartupFieldParameters> = this.get_widget_html_controller();
        
        for(const [field_name, field] of Object.entries(response)) {
            const field_action: FieldAction = widget_html_controller.determine_creationary_action(field_name, field);
            switch(field_action) {
                case FieldAction.Create:
                    var new_child: BaseWidget = new BaseWidget(
                        this.generate_html(field_name),
                        new BaseWidgetMetadata(
                            field_name, 
                            new WidgetParent(widget_html_controller.get_node())
                        )
                    );
                    widget_html_controller.add_child(new_child);
                    break;
                case FieldAction.Update:
                    widget_html_controller.update_child(field_name, field);
                    break;
            }
        }

        widget_html_controller.delete_necessary(Object.keys(response));
    }
}