import { HTMLControllerError, WidgetError } from "../exceptions/module_errors";
import { BaseWidget } from "../widgets/widget/widget";

export class Page {
    available_fields: Map<string, HTMLElement> = new Map<string, HTMLElement>();
    occupied_fields: Map<string, HTMLElement> = new Map<string, HTMLElement>();

    constructor() {
        var elements_accepting_widgets: NodeListOf<HTMLElement> = document.querySelectorAll('[data-isfield="true"]');
        
        for(const element of elements_accepting_widgets) {
            console.log(element.id)
            if(element.id === "" || !element.id) {
                throw new HTMLControllerError("HTML_TEMPLATE_ERROR", 
                    `Page field lacks an ID, make sure all elements with the data-isfield="true" attribute also have IDs`, this)
            }
            this.available_fields.set(element.id, element);
        }
    }

    public consume_element(id: string): HTMLElement {
        var field_requested: HTMLElement | undefined = this.available_fields.get(id);
        if(!field_requested) {
            var field_requested_occupied: HTMLElement | undefined = this.occupied_fields.get(id);

            if(field_requested_occupied) {
                throw new WidgetError("IMPROPER_CONFIGURATION_ERROR", 
                    `The page field ${id} has already been consumed by another widget`, this)
            }

            else {
                throw new WidgetError("IMPROPER_CONFIGURATION_ERROR", 
                    `The page field ${id} does not exist on the page!`, this)
            }
        }

        this.available_fields.delete(id);
        this.occupied_fields.set(id, field_requested);

        return field_requested;
    }

    public get_available_fields(): Map<string, HTMLElement> {
        return this.available_fields;
    }
}