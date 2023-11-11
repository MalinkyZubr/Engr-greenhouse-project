import { asynchronous_updater, get_project_associated_devices, format_route } from "./shared/asynchronousUpdater.js"

let re = new RegExp("(?<=Project: ).*");

let project_name = re.exec(document.getElementById("project_name").textContent)[0];

let interval_regex = new RegExp("(?!00)\\d{2}-(?!00)\\d{2}-(?!00)\\d{2}");
let REGEX_ERROR = "must be mm-dd-yy"

class ProjectManager {
    constructor() {
        // declare the variables for datatype, data retrieval technique, and date interval
        this.data_type = "Temperature";
        this.update_type = "live";
        this.date_interval = [0, 0];

        this.data_type_radio_buttons = document.getElementsByName('data-selected');

        this.interval_display = document.getElementById("display-interval");
        this.lower_interval_input = this.interval_display.querySelector("#start-interval"); 
        this.upper_interval_input = this.interval_display.querySelector("#end-interval");
        this.refresh_button = this.interval_display.querySelector("#image-refresh");
        this.refresh_button.addEventListener("click", () => this.get_graph_image(project_name, false));

        this.lower_interval_input.value = "";
        this.upper_interval_input.value = "";

        this.interval_display.style.display="none";

        this.data_view_div = document.getElementById("data-view-type");
        this.data_view_buttons = document.getElementsByName("update-type");

        this.csv_download_button = document.getElementById("download-csv");
        this.pdf_download_button = document.getElementById("download-pdf");

        this.add_device_button = document.getElementById("add-device");
        this.add_device_button.addEventListener("click", async function() {
            document.location.href = await format_route(`/devices/scan?=${project_name}`);
        });

        this.set_data_type_events();
        this.set_interval_events();
        this.set_view_events();
        this.configure_download_buttons(project_name);
    }

    configure_download_buttons(project_name) {
        this.csv_download_button.addEventListener('click', async function() {
            await this.download_csv(project_name);
        }.bind(this));
    }

    async download_csv(project_name) {
        console.log("CALLING HERE");
        var csv_url = await format_route(`/projects/projects/${project_name}/download_csv`);
        console.log(csv_url);
        var download_link = document.createElement('a');
        download_link.href = csv_url;
        download_link.download = `${project_name}.csv`;

        document.body.appendChild(download_link);
        download_link.click();
        document.body.removeChild(download_link);
    }

    set_data_type_events() {
        for(let x = 0; x < this.data_type_radio_buttons.length; x++) {
            this.data_type_radio_buttons[x].addEventListener('change', this.data_type_event_handler.bind(this)); // the bind makes sure that the anonymous function can access instance variables
        }
    }

    data_type_event_handler() {
        for(let x = 0; x < this.data_type_radio_buttons.length; x++) {
            if(this.data_type_radio_buttons[x].checked) {
                this.data_type = this.data_type_radio_buttons[x].value;
                break;
            }
        }
    }

    set_interval_events() {
        this.lower_interval_input.addEventListener('input', function() {
            this.interval_change_event_handler(this.lower_interval_input, "start-interval-error", 0);
        }.bind(this))
        this.upper_interval_input.addEventListener('input', function() {
            this.interval_change_event_handler(this.upper_interval_input, "end-interval-error", 1);
        }.bind(this))
    }

    check_match_date(lower_date, upper_date) {
        if(interval_regex.test(lower_date) && interval_regex.test(upper_date)) {
            this.refresh_button.style.display="block";
        }
        else {
            this.refresh_button.style.display="none";
        }
    }

    interval_change_event_handler(interval_input, interval_input_error_id, is_end_interval) {
        if(interval_input.value != "" && !interval_regex.test(interval_input.value)) {
            this.interval_display.querySelector(`#${interval_input_error_id}`).textContent=REGEX_ERROR;
        }
        else {
            this.interval_display.querySelector(`#${interval_input_error_id}`).textContent='';
            this.date_interval[is_end_interval] = interval_input.value;
        }
        this.check_match_date(this.lower_interval_input.value, this.upper_interval_input.value);
        console.log(this.date_interval);
    }

    set_view_events() {
        for(let x = 0; x < this.data_view_buttons.length; x++) {
            this.data_view_buttons[x].addEventListener('change', this.view_change_event.bind(this));
        }
    }

    view_change_event() {
        for(let x = 0; x < this.data_view_buttons.length; x++) {
            if(this.data_view_buttons[x].checked && this.data_view_buttons[x].value === "hide") {
                this.interval_display.style.display="none";
                this.date_interval = [0, 0];
                this.lower_interval_input.value = "";
                this.upper_interval_input.value = "";
                this.update_type = 'live';
                break;
            }
            else {
                this.interval_display.style.display="block";
                this.update_type = 'static';
                break;
            }
        }
    }

    async get_graph_image(project_name, in_loop=true) {
        var route = await format_route(`/projects/projects/${project_name}/data_visualization/${this.data_type}`);
        if((this.update_type === "live" && in_loop) || (this.update_type === "static" && !in_loop)) {
            await fetch(route, {
                method: 'POST',
                headers: {
                    'Content-Type':'application/json'
                },
                body: JSON.stringify({
                    start_date:this.date_interval[0],
                    end_date:this.date_interval[1]
                })
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

    async get_data_csv(project_name) {
        var route = await format_route(`/projects/projects/${project_name}/download_csv`)
        await fetch(route, {
            method: 'GET'
        })
        .then(response => response.text())
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