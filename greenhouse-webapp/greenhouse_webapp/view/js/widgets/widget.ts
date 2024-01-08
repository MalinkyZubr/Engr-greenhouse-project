const find_template_params: string = "(?<=\\{{ )(.*?)(?=\\ }})";
const find_template_id_fields: string = "(?<=id=)['\"]?([^'\"\\s]+)['\"]?"
const extract_from_quotes: string = "(?<=')[^']+?(?=')"


export class WidgetParentData {
    private parent_element: HTMLElement;

    constructor(parent_id: string) {
        this.parent_element = document.getElementById(parent_id) ?? function() { throw new Error(`${parent_id} id not found for parent`) }();
    }

    public get_parent_element(): HTMLElement {
        return this.parent_element;
    }
}

export abstract class Widget {
    public static widget_run_list: Array<Widget> = new Array<Widget>();
    abstract widget_html: string;
    abstract widget_name: string;

    private template_static_fields: RegExpMatchArray | null;
    private template_dynamic_fields: RegExpMatchArray | null;
    
    private widget_data: object;

    private widget_node: HTMLElement;
    private widget_parent_node: HTMLElement

    constructor(widget_data: object, parent_data: WidgetParentData) { // should only call on dom loaded event
        if(!this.get_widget_html().includes("id={{ element_id }}")) {
            throw new Error(`The html for the widget ${this.get_widget_name()} with id ${widget_data["element_id"]} does not have an element_id field`)
        }
        this.widget_parent_node = parent_data.get_parent_element();

        this.template_static_fields = this.locate_static_template_fields();
        this.template_dynamic_fields = this.locate_dynamic_template_fields();

        this.set_widget_html(this.return_formatted_html(widget_data, this.get_widget_html()));

        this.widget_node = this.insert_widget_to_parent_page();

        this.update(widget_data);

        Widget.widget_run_list.push(this);
    }

    public static async run_all(): Promise<void> {
        for(const widget_index in Widget.widget_run_list) {
            const widget: Widget = Widget.widget_run_list[widget_index];
            await widget.run();
        }
    }

    public get_id(): string {
        return this.get_widget_data()["element_id"];
    }

    private locate_static_template_fields(): RegExpMatchArray | null {
        return this.widget_html.match(find_template_params);
    } 

    private locate_dynamic_template_fields(): RegExpMatchArray | null {
        var dynamic_template_fields: RegExpMatchArray | null = this.widget_html.match(find_template_id_fields);

        if(dynamic_template_fields) {
            for(var x: number = 0; x < dynamic_template_fields?.length; x++) {
                var dequoted_element: RegExpMatchArray | null= dynamic_template_fields[x].match(extract_from_quotes)
                if(dequoted_element) {
                    dynamic_template_fields[x] = dequoted_element[0];
                }
            }
        }

        return dynamic_template_fields;
    }

    private insert_template_static_field(key: string, value: string, html: string): string {
        html = html.replace(`{{ ${key} }}`, value);
        return html;
    }

    private return_formatted_html(widget_data: object, unformatted_html: string): string {
        var formatted_html: string = unformatted_html.substring(0);

        if(!this.template_static_fields && Object.keys(widget_data).length > 0) {
            throw new Error(`The widget: ${this.widget_name}, takes no parameters`);
        }
        else if(!this.template_static_fields) {
            return formatted_html;
        }
        for(const key in this.template_static_fields) {
            if(!(key in Object.keys(widget_data)) && (this.template_dynamic_fields && !(key in this.template_dynamic_fields))) {
                throw new Error(`The widget: ${this.widget_name}, requires a value for ${key}`);
            }
        }
        for(const [key, value] of Object.entries(widget_data)) {
            if(!(key in this.template_static_fields)) {
                throw new Error(`The widget: ${this.widget_name}, does not take a value for ${key}`)
            }
            this.insert_template_static_field(key, value, formatted_html);
        }
        return formatted_html;
    }

    public update(to_update: object): void {
        this.widget_data = to_update;

        for(const field in this.template_dynamic_fields) {
            var field_to_update: Element | null = this.get_widget_node().querySelector(`#${field}`);

            if(!field_to_update || !to_update[field]) {
                continue;
            }

            field_to_update.innerHTML = to_update[field];
        }
    }

    public insert_widget_to_parent_page(): HTMLElement {
        var new_widget: HTMLElement = document.createElement("div");
        var widget_content: Text = document.createTextNode(this.widget_html);

        new_widget.appendChild(widget_content);
        
        this.get_widget_parent().appendChild(new_widget);
        return this.get_widget_parent().querySelector(`#${this.get_id()}`) ?? function() { throw new Error(`${this.get_id()} not found`) }();
    }

    private set_widget_html(html: string): void {
        this.widget_html = html;
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

    public async run(): Promise<void> {}
}