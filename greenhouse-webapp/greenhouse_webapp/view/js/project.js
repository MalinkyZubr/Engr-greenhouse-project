import { asynchronous_updater, get_project_associated_devices, format_route } from "./shared/asynchronousUpdater.js"

let re = new RegExp("(?<=Project: ).*");

let project_name = re.exec(document.getElementById("project_name").textContent)[0];

let interval_regex = new RegExp("\d\d-\d\d-\d\d");

class ProjectManager {
    constructor() {
        this.data_type = "Temperature";
        this.update_type = "live";
        this.date_interval = [null, null];

        this.data_type_div = document.getElementById("data-view-type");
        this.data_type_radio_buttons = this.data_type_div.querySelector('input[name="data-selected"]');

        this.interval_display = document.getElementById("display-interval");
        this.lower_interval_input = this.interval_display.querySelector("#start-interval"); 
        this.upper_interval_input = this.interval_display.querySelector("#end-interval");
        this.refresh_button = this.interval_display.querySelector("#image-refresh");
        this.refresh_button.addEventListener("click", () => this.get_graph_image(project_name, false));

        this.lower_interval_input.style.visibility='hidden';
        this.upper_interval_input.style.visibility='hidden';

        this.data_view_div = document.getElementById("data-view-type");
        this.data_view_buttons = this.data_view_div.querySelector('input[name="update-type"]');

        this.set_data_type_events();
        this.set_interval_events();
        this.set_view_events();
    }

    set_data_type_events() {
        for(let x = 0; x < this.data_type_radio_buttons.length; x++) {
            this.data_type_radio_buttons[x].addEventListener('change', this.data_type_event_handler);
        }
    }

    data_type_event_handler() {
        for(let x = 0; x < this.data_type_radio_buttons.length; x++) {
            if(this.data_type_radio_buttons[x].checked) {
                this.data_type = data_type_radio_buttons[x].value;
                break;
            }
        }
    }

    set_interval_events() {
        this.lower_interval_input.addEventListener('change', function() {
            this.interval_change_event_handler(this.lower_interval_input, "start-interval-error", 0);
        })
        this.upper_interval_input.addEventListener('change', function() {
            this.interval_change_event_handler(this.upper_interval_input, "end-interval-error", 1);
        })
    }

    interval_change_event_handler(interval_input, interval_input_error_id, is_end_interval) {
        if(interval_input.value != "" && !interval_regex.test(interval_input.value)) {
            this.interval_display.querySelector(`#${interval_input_error_id}`).style.visibility='visible';
        }
        else {
            this.interval_display.querySelector(`#${interval_input_error_id}`).style.visibility='hidden';
            this.date_interval[is_end_interval] = interval_input.value;
        }
    }

    set_view_events() {
        for(let x = 0; x < this.data_view_buttons.length; x++) {
            this.data_view_buttons.addEventListener('change', this.view_change_event);
        }
    }

    view_change_event() {
        for(let x = 0; x < this.data_view_buttons.length; x++) {
            if(this.data_view_buttons[x].checked && this.data_view_buttons[x].value === "hide") {
                this.interval_display.style.visibility="hidden";
                this.date_interval = [null, null];
                this.update_type = 'live';
            }
            else {
                this.interval_display.style.visibility="visible";
                this.update_type = 'static';
            }
        }
    }

    async get_graph_image(project_name, in_loop=true) {
        var route = await format_route(`/projects/projects/${project_name}/data_visualization/${this.data_type}`);
        if((this.update_type === "live" && in_loop) || (this.update_type === "static" && !in_loop)) {
            await fetch(route, {
                method: 'GET',
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify(this.date_interval)
            })
            .then(response => response.blob())
            .then(blob =>
                {
                    const imageUrl = URL.createObjectURL(blob);
                    const image_element = document.getElementById("graph-image");
                    image_element.src = imageUrl;
                }    
            )
            .catch(error => console.error('Error fetching .png file:', error));
        }
    }
}

let project_manager = new ProjectManager();

let functions = [
    async function() {
        get_project_associated_devices(project_name);
    },
    async function() {
        project_manager.get_graph_image(project_name);
    }
];

asynchronous_updater(functions);