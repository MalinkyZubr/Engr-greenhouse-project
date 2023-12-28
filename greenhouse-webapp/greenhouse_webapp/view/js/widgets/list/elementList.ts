import { StandardWidget, Widget } from "../widget.ts"

export interface ObjectTemplateParameters {
    item_id: string | number;
}

export abstract class ListElement extends Widget {
    abstract widget_html: string;

    constructor(list_element_data: object, parent_list: HTMLElement) {
        super(list_element_data, parent_list);
    }
    
    public insert_widget_to_parent_page(element_id: string): HTMLElement {
        var html_object_widget: HTMLDivElement = document.createElement("div");
        html_object_widget.innerHTML = this.get_widget_html();

        this.get_widget_parent().appendChild(html_object_widget);
        return this.get_element_by_id(element_id);
    }

    public equals(widget_data: object): boolean {
        for(const field in widget_data) {
            if(this.get_widget_data[field] != widget_data[field]) {
                return false;
            }
        }
        return true;
    }
}

export abstract class List extends StandardWidget {
    public widget_html: string = 
        `<div id={{ element_id }}>
            <div class="header">
                <h2>{{ list_name }}</h2>
            </div>
            <div class="table">
                <table class="table">
                    <thead>
                        <tr>{{ table_header }}</tr>
                    </thead>
                    <tbody id="table">
                        <!-- rows of stuff will go here -->
                    </tbody>
                </table>
            </div>
        </div>`
    private node_object_list: Map<string | number, ListElement>;

    public constructor(list_data: object, parent_element: HTMLElement, widget_key: string) {
        super(list_data, parent_element, widget_key)
        this.node_object_list = new Map<string | number, ListElement>();
    }

    public get_list_element(element_id: string): object | undefined {
        var element: ListElement | undefined = this.node_object_list.get(element_id);

        if(element) {
            return element.get_widget_data();
        }
        return element;
    }

    public append(widget_data: object): void {
        var table: HTMLElement | null = this.get_widget_node().querySelector(`#table`);
        if(table) {
            var list_element: ListElement = this.create_list_element(widget_data, table)
            this.node_object_list.set(list_element.get_element_id(), list_element);
        }
    }

    private frontend_remove(node_id: string | number): void {
        var list_element: ListElement | undefined = this.node_object_list["node_id"];
        if(!list_element) {
            throw new Error(`The element ${node_id} was not found`);
        }
        this.get_widget_parent().removeChild(list_element.get_widget_node());
    }

    public remove(id: string | number) : void {
        this.node_object_list.delete(id);
        this.frontend_remove(id);
    }

    public update_element(id: string, widget_data: object) : void {
        var selected_element: ListElement | undefined = this.node_object_list.get(id);

        if(selected_element) {
            selected_element.update(widget_data);
        }
    }

    public handle_response(object_map: Map<string, object>) {
        for(const [item_id, item_object] of this.node_object_list.entries()) {
            if(!(item_id in object_map.keys())) {
                this.remove(item_id);
            }
        }

        for(const [item_id, item_object] of object_map.entries()) {
            if(!(item_id in this.node_object_list.keys())) {
                this.append(item_object);
            }
            else if(this.node_object_list.get(item_id)?.equals(item_object)) {
                this.update_element(item_id, item_object);
            }
        }
    }

    abstract create_list_element(element_data: object, this_list_html: HTMLElement): ListElement;
}

export async function request_server_list_data(host: string, route: string): Promise<Map<string, object> | null> {
    var parameter_object_list: Map<string, object> | null = new Map<string, object>();

    const response: Response = await fetch(`http://${host}${route}`)
    const json_data: JSON = await response.json();

    if(!Array.isArray(json_data)) {
        throw new Error("Expected data in an array format!")
    }
    else {
        for(var i: number = 0; i < json_data.length; i++) {
            var parameter_object: object = await json_data[i].parse()
            parameter_object_list[parameter_object[this.item_id_name]] = parameter_object;
        }
    }
    if(parameter_object_list.size === 0) {
        parameter_object_list = null;
    }
    return parameter_object_list
}
