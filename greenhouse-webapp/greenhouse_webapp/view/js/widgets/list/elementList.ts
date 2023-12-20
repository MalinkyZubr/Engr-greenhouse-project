import { Widget } from "../widget.ts"

export interface ObjectTemplateParameters {
    item_id: string | number;
}

export abstract class ListElement extends Widget{
    constructor(list_element_data: object, parent_list: HTMLElement) {
        super(list_element_data, parent_list);
    }

    public generate_html(): string {
        var object_template_copy: string = this.object_html_template.substring(0, this.object_html_template.length);
        var modified: string = object_template_copy;

        for(const field in this.parameters) {
            modified = object_template_copy.replace("{{ " + field + " }}", String(this.parameters[field]));
            if(modified === object_template_copy) {
                throw new Error("The parameter was not found in the template");
            }
            object_template_copy = modified;
        }

        object_template_copy = `<tr><td>${object_template_copy}</td></tr>`;
        return object_template_copy;
    }

    public get_parameters(): P {
        return this.parameters;
    }

    public update(parameters: P): void {
        this.parameters = parameters;
    }

    public equals(parameters: P): boolean {
        for(const field in parameters) {
            if(this.parameters[field] != parameters[field]) {
                return false;
            }
        }
        return true;
    }
}

export abstract class List<P extends ObjectTemplateParameters, T extends ObjectTemplate<P>> {
    private parent_list: HTMLElement;
    private node_object_list: Map<string | number, T>;

    public constructor(parent_list: HTMLElement) {
        this.parent_list = parent_list;
        this.node_object_list = new Map<string | number, T>();
    }

    public set_parent_list(parent_list: HTMLElement) {
        this.parent_list = parent_list;
    }

    private get_node(id: string | number) : Element {
        var list_element: Element | null | undefined = this.parent_list.querySelector(`#${String(id)}`);
        if(list_element === null || list_element === undefined) {
            throw new Error("The node does not exist")
        }
        return list_element;
    }

    private frontend_append(node_object: T): void {
        var text_node: Text = document.createTextNode(node_object.generate_html());
        
        this.parent_list.appendChild(text_node);
    }

    public append(parameters: P): void {
        var node_object: T = this.create_node(parameters);
        this.node_object_list.set(node_object.get_parameters().item_id, node_object);

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

    abstract object_to_parameters(obj: object): P;
    abstract create_node(parameters: P): T;
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
