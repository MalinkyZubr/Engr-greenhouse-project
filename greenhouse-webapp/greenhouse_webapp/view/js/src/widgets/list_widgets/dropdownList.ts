import { AbstractBaseWidgetHTMLController } from "../widget/dynamic/widget_html";
import { BaseStartupFieldParameters } from "../widget/dynamic/widget_html";
import { DropdownOptionController, DropdownOptionStartupFieldParameters } from "../misc_inputs/dropdownOption";
import { ListAppendModule, WidgetListHTMLController } from "../widget/list_widget";
import { FieldParameters } from "../widget/dynamic/field_container";


export class DropdownListHTMLController extends WidgetListHTMLController<BaseStartupFieldParameters> {
    private selection_node: HTMLSelectElement | null = null;

    public html_template_generator(): string {
        return `<select id={{ element_id }}><select>`;
    }

    public assign_widget_node(widget_node: HTMLElement): AbstractBaseWidgetHTMLController<BaseStartupFieldParameters> {
        super.assign_widget_node(widget_node);

        var selection_node: HTMLSelectElement | null = this.get_node()?.querySelector(`#${this.get_id()}`);
        
        if(!selection_node) {
            throw new Error(`The DropdownList controller ${this.constructor.name} has no selection structure ${this.get_id()}`);
        }
        this.selection_node = selection_node;

        return this
    }

    public get_value(): FieldParameters {
        var value: string | undefined = this.selection_node?.value

        if(!value) {
            throw new Error(`dropdown ${this.constructor.name} does not have a value`);
        }

        return {"value":value};
    }
}


export abstract class DropdownListAppendModule extends ListAppendModule<DropdownOptionController> {
    public generate_html(element_id: string, element_fields: FieldParameters): DropdownOptionController {
        var dropdown_option_html: DropdownOptionController = new DropdownOptionController(
            new DropdownOptionStartupFieldParameters(element_id, element_id, element_fields),
        )

        return dropdown_option_html;
    }
}