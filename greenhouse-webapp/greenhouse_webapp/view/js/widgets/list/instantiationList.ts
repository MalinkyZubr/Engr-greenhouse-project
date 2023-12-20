import { ObjectTemplateParameters, ObjectTemplate, ElementList, request_server_data } from "./elementList.ts";
import { PeriodicExecutor } from "../../shared/periodic.ts";


try {
    let device_name = document.getElementById("device_name");
}
catch {
    let device_name = null;
}


class InstantiationListObjectTemplate<P extends ObjectTemplateParameters> extends ObjectTemplate<P> {
    object_html_template: string = 
        `<tr>
            <td>
                <label class="radio-button" id="radio-label">
                    <input type="radio" name="options" value=1>
                    <span class="radiomark"></span>
                    {{ item_id }}
                </label>
            </td>
        </tr>`;
}


export abstract class InstantiationList<P extends ObjectTemplateParameters> extends PeriodicExecutor {
    private buttons_list: NodeListOf<HTMLElement>;
    private assignment_button: HTMLElement;
    private received_object_list: ElementList<P, InstantiationListObjectTemplate<P>>;

    abstract host: string;
    abstract route: string;

    constructor(received_object_list: ElementList<P, InstantiationListObjectTemplate<P>>) { // assignment type is either preset_name or project_name
        super(10000);

        this.received_object_list = received_object_list;

        var selector_table: HTMLElement | null = document.getElementById("selector-table");
        if(!selector_table) {
            throw new Error("selector-table id node not found in HTML document!");
        }

        this.received_object_list.set_parent_list(selector_table);

        this.buttons_list = document.getElementsByName("options");

        var assignment_button: null | HTMLElement = document.getElementById("assign");
        if(assignment_button) {
            this.assignment_button = assignment_button;
            this.assignment_button.addEventListener("click", this.instantiate_button_callback);
        }
        else {
            throw new Error("Assignment button not found");
        }
    }

    abstract instantiate_button_callback(): void;

    async call_methods(): Promise<void> {
        var object_list: Map<string | number, object> | null = await request_server_data(this.host, this.route);
        if(object_list) {
            this.received_object_list.handle_response(object_list);
        }
    }
}