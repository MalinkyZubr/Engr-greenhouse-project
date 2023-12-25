const find_template_params: string = "(?<=\{{ )(.*?)(?=\ }})";

export abstract class Widget {
    abstract widget_html: string;
    abstract widget_name: string;
    private template_parameters: RegExpMatchArray | null;
    private widget_data: object;

    private widget_node: HTMLElement;
    private widget_parent_node: HTMLElement

    constructor(widget_data: object, parent_element: HTMLElement, widget_key?: string) { // should only call on dom loaded event
        var formatted_html: string;

        if(!this.get_widget_html().includes("id={{ element_id }}")) {
            throw new Error(`The html for the widget ${this.get_widget_name()} with id ${widget_data["element_id"]} does not have an element_id field`)
        }
        this.widget_data = widget_data;
        this.widget_parent_node = parent_element;

        this.template_parameters = this.locate_template_parameters();
        formatted_html = this.return_formatted_html(widget_data, this.get_widget_html(), this.get_element_id());

        if(widget_key) {
            this.widget_node = this.insert_widget_to_parent_page(formatted_html, this.get_element_id(), widget_key);
        }
        else {
            this.widget_node = this.insert_widget_to_parent_page(formatted_html, this.get_element_id());
        }
    }

    public get_element_id(): string {
        return this.get_widget_data()["element_id"];
    }

    private locate_template_parameters(): RegExpMatchArray | null {
        return this.widget_html.match(find_template_params);
    } 

    private insert_template_parameter(key: string, value: string, html: string): string {
        html.replace(`{{ ${key} }}`, value);
        return html;
    }

    private return_formatted_html(widget_data: object, unformatted_html: string, element_id: string): string {
        var formatted_html: string = unformatted_html.substring(0);

        if(!this.template_parameters && Object.keys(widget_data).length > 0) {
            throw new Error(`The widget: ${this.widget_name}, takes no parameters`);
        }
        else if(!this.template_parameters) {
            return formatted_html;
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
            this.insert_template_parameter(key, value, formatted_html);
        }
        return formatted_html;
    }

    abstract insert_widget_to_parent_page(widget_html: string, element_id: string, widget_key?: string): HTMLElement;

    public get_element_by_id(element_id: string) {
        var html_widget_node: HTMLElement | null = this.get_widget_parent().querySelector(`#${element_id}`);
        if(!html_widget_node) {
            throw new Error(`${element_id} was not found on the page despite being inserted`);
        }
        return html_widget_node;
    }

    public get_widget_html(): string {
        return this.widget_html;
    }

    public get_widget_data(): object {
        return this.widget_data;
    }

    public get_widget_name(): string {
        return this.widget_name;
    }

    public get_widget_parent(): HTMLElement {
        return this.widget_parent_node;
    }

    public get_widget_node(): HTMLElement {
        return this.widget_node;
    }
}

export abstract class StandardWidget extends Widget {
    abstract widget_html: string;
    abstract widget_name: string;

    public insert_widget_to_parent_page(widget_html: string, element_id: string, widget_key: string): HTMLElement {
        var parent_inner_html: string = this.get_widget_parent().innerHTML;
        var new_parent_inner_html: string = parent_inner_html.replace(`{{ ${widget_key} }}`, widget_html);

        if(parent_inner_html === new_parent_inner_html && widget_key) {
            throw new Error(`The specified key for the widget ${this.widget_name}: ${widget_key} does not exist in parent HTML`);
        }
        else if(parent_inner_html === new_parent_inner_html) {
            throw new Error(`The specified `)
        }

        this.get_widget_parent().innerHTML = new_parent_inner_html;

        return this.get_element_by_id(element_id);
    }
}