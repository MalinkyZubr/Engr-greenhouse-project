const find_template_params: string = "(?<=\{{ )(.*?)(?=\ }})";

export abstract class Widget {
    abstract widget_html: string;
    abstract widget_name: string;
    private template_parameters: RegExpMatchArray | null;
    private widget_data: object;
    private widget_parent: HTMLElement

    constructor(widget_data: object, parent_element: HTMLElement, element_id: string, widget_key?: string) { // should only call on dom loaded event
        if(!this.get_widget_html().includes("id={{ element_id }}")) {
            throw new Error(`The html for the widget ${this.get_widget_name()} with id ${element_id} does not have an element_id field`)
        }
        this.widget_data = widget_data;
        this.widget_parent = parent_element;

        this.template_parameters = this.locate_template_parameters();
        this.load_widget_data(widget_data);

        if(widget_key) {
            this.insert_widget_to_parent_page(parent_element, this.get_widget_html(), widget_key);
        }
        else {
            this.insert_widget_to_parent_page(parent_element, this.get_widget_html());
        }
    }

    private locate_template_parameters(): RegExpMatchArray | null {
        return this.widget_html.match(find_template_params);
    } 

    private insert_template_parameter(key: string, value: string): void {
        this.widget_html.replace(`{{ ${key} }}`, value);
    }

    private load_widget_data(widget_data: object): void {
        if(!this.template_parameters && Object.keys(widget_data).length > 0) {
            throw new Error(`The widget: ${this.widget_name}, takes no parameters`);
        }
        else if(!this.template_parameters) {
            return;
        }
        for(const key in this.template_parameters) {
            if(!(key in Object.keys(widget_data))) {
                throw new Error(`The widget: ${this.widget_name}, requires a value for ${key}`);
            }
        }
        for(const [key, value] of Object.entries(widget_data)) {
            if(!(key in this.template_parameters)) {
                throw new Error(`The widget: ${this.widget_name}, does not take a value for ${key}`)
            }
            this.insert_template_parameter(key, value);
        }
    }

    abstract insert_widget_to_parent_page(parent_element: HTMLElement, widget_html: string, widget_key?: string): void;

    public get_widget_html(): string {
        return this.widget_html;
    }

    public get_widget_data(): object {
        return this.widget_data;
    }

    public get_widget_name(): string {
        return this.widget_name;
    }
}

export abstract class StandardWidget extends Widget {
    abstract widget_html: string;
    abstract widget_name: string;

    public insert_widget_to_parent_page(parent_element: HTMLElement, widget_html: string, widget_key: string): void {
        var parent_inner_html: string = parent_element.innerHTML;
        var new_parent_inner_html: string = parent_inner_html.replace(`{{ ${widget_key} }}`, widget_html);

        if(parent_inner_html === new_parent_inner_html && widget_key) {
            throw new Error(`The specified key for the widget ${this.widget_name}: ${widget_key} does not exist in parent HTML`);
        }
        else if(parent_inner_html === new_parent_inner_html) {
            throw new Error(`The specified `)
        }

        parent_element.innerHTML = new_parent_inner_html;
    }
}