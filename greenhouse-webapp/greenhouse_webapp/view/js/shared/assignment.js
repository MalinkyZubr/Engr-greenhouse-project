import { asynchronous_updater, format_route } from "./asynchronousUpdater.js";

try {
    let device_name = document.getElementById("device_name");
}
catch {
    let device_name = null;
}


export class Assigner {
    constructor(assignment_type) { // assignment type is either preset_name or project_name
        this.button_template = document.getElementById("assign-template");
        this.selector_table = document.getElementById("selector-table");

        this.buttons = document.getElementsByName("options");

        this.assignment_type = assignment_type

        this.assignment_button = document.getElementById("assign");
        if(this.assignment_type != "archived") {
            this.assignment_button.addEventListener("click", this.assign_to_device)
        }
        else {
            this.assignment_button.addEventListener("click", this.load_archive_project)
        }
    }

    async retrieve_elements() {
        if(this.assignment_type == "preset_name") {
            var route = `/projects/projects/list_projects`;
        }
        else if(this.assignment_type == "project_name") {
            var route = `/presets/list_presets`;
        }
        else {
            var route = `/projects/archived/projects`
        }
        route = await format_route(route)
        .then(res => res.json())
        .then(json_res => {
            for(let data in json_res) {
                button = document.importNode(this.button_template.content, true);
                button_content = button.querySelector("#radio-label");
                button_content.textContent = data[this.assignment_type];
                this.selector_table.append(button);
            }
        })
        this.buttons = document.getElementsByName("options");
    }

    async assign_to_device() {
        for(let button in this.buttons) {
            if(button.checked) {
                var selected_element_content = button.textContent;
                var body = {};
                body[this.assignment_type] = selected_element_content;
                
                route = await format_route(`/devices/devices/${device_name}/update`);
                await fetch(route, {
                    method: "PUT",
                    headers: {
                        "Content-Type":"application/json"
                    },
                    body: JSON.stringify(body)
                })
                .then(res => res.json());
                
                route = await format_route(`/devices/devices/${device_name}`);
                await fetch(route);

                return;
            }
        }
        alert(`please select one to assign to ${device_name}`);
    }

    async load_archive_project() {
        for(let button in this.buttons) {
            if(button.checked) {
                var selected_element_content = button.textContent;
                
                route = await format_route(`/projects/archived/${selected_element_content}`);
                await fetch(route);

                return;
            }
        }
        alert(`please select one to assign to view!`);
    }
}