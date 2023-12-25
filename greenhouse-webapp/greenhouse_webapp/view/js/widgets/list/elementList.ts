import { StandardWidget, Widget } from "../widget.ts"

export interface ObjectTemplateParameters {
    item_id: string | number;
}

export abstract class ListElement extends Widget{
    abstract widget_html: string;

    constructor(list_element_data: object, parent_list: HTMLElement) {
        super(list_element_data, parent_list);
    }
    
    public insert_widget_to_parent_page(widget_html: string, element_id: string): HTMLElement {
        var html_object_widget: HTMLDivElement = document.createElement("div");
        html_object_widget.innerHTML = widget_html;

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

    private get_node(id: string | number) : Element {
        var list_element: Element | null | undefined = this.get_widget_parent().querySelector(`#${String(id)}`);
        if(list_element === null || list_element === undefined) {
            throw new Error("The node does not exist")
        }
        return list_element;
    }

    public append(widget_data: object): void {
        var list_element: ListElement = this.create_list_element(widget_data, this.get_widget_parent())
        this.node_object_list.set(list_element.get_element_id(), list_element);

        this.frontend_append(node_object);
    }

    private frontend_remove(node_id: string | number): void {
        var list_element: Element = this.get_node(node_id);
        this.parent_list.removeChild(list_element);
    }

    public remove(id: string | number) : void {
        this.node_object_list.delete(id);
        this.frontend_remove(id);
    }

    public update(id: string | number, parameters: P) : void {
        this.frontend_remove(id);
        var node_object: T | null = this.node_object_list.get(id) ?? null;
        if(!node_object) {
            throw new Error("Node not found when updating");
        }
        node_object?.update(parameters);
        this.frontend_append(node_object);
    }

    public handle_response(object_map: Map<string | number, object>) {
        var parameters_list: Map<string | number, P> = this.convert_objects_to_parameters(object_map);
        for(const item_id in this.node_object_list.keys()) {
            if(!(item_id in parameters_list.keys())) {
                this.remove(item_id);
            }
        }
        for(const [item_id, item_object] of parameters_list.entries()) {
            var current_id: string = String(item_id);
            if(!(current_id in this.node_object_list.keys())) {
                this.append(item_object);
            }
            else if(!this.node_object_list.get(current_id)?.equals(item_object)) {
                this.update(current_id, item_object);
            }
        }
    }

    private convert_objects_to_parameters(objects: Map<string | number, object>): Map<string | number, P> {
        var parameters_list: Map<string | number, P> = new Map<string | number, P>();
        for(const [key, value] of objects.entries()) {
            parameters_list[key] = this.object_to_parameters(value);
        }
        return parameters_list;
    }  

    abstract create_list_element(element_data: object, this_list_html: HTMLElement): ListElement;
}

export async function request_server_data(host: string, route: string): Promise<Map<string | number, object> | null> {
    var parameter_object_list: Map<string | number, object> | null = new Map<string | number, object>();

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
