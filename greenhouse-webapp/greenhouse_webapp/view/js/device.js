import { asynchronous_updater, format_route } from "./shared/asynchronousUpdater.js";


let device_name = document.getElementById("device_name").textContent;


class DescriptiveStats {
    constructor() {
        this.stats_table = document.getElementById("stats");
        this.fields = {
            uptime : this.stats_table.querySelector("#uptime"),
            temperature : this.stats_table.querySelector("#temperature"),
            humidity : this.stats_table.querySelector("#humidity"),
            moisture : this.stats_table.querySelector("#moisture"),
            ph : this.stats_table.querySelector("#ph"),
            light : this.stats_table.querySelector("#light"),
        };
    }

    change_fields(data) {
        for(let datapoint_key in data.keys()) {
            this.fields[datapoint_key] = data[datapoint_key];
        }
    }
}


class DeviceLogging {
    constructor() {
        this.log_table = document.getElementById("logging-output");
        this.log_template = document.getElementById("log-template");
        this.download_logs_button = document.getElementById("log-button");

        this.download_logs_button.addEventListener("click", async function() {
            await this.download_logs();
        }.bind(this));
    }

    print_log(log_data) {
        var log_entry = document.importNode(this.log_template.content, true);
        var log_content = log_entry.querySelector("#log-message");

        log_content.textContent = `${log_data['log_timestamp']} - ${log_data['log_level']} : ${log_data['log_content']}`;
        this.log_table.append(log_entry);
    }

    async retrieve_logs() {
        route = format_route(`/devices/devices/${device_name}/logs`);
        this.log_table.innerHTML = "";

        await fetch(route)
        .then(res => res.json())
        .then(json_response => {
            for(let log in json_response) {
                this.print_log(log_data);
            }
        })
    }

    async download_logs() {
        route = format_route(`/devices/devices/${device_name}/downloadLogs`);

        await fetch(route)
        .then(response => response.text());
    }
}


class DeviceIdentifiers {
    constructor() {
        this.identifiers_table = document.getElementById("device-identifiers");
        this.static_fields = {
            preset_name : this.identifiers_table.querySelector("preset_name"),
            project_name : this.identifiers_table.querySelector("project_name"),
            device_ip : this.identifiers_table.querySelector("ip"),
            device_mac : this.identifiers_table.querySelector("mac"),
            registration_time : this.identifiers_table.querySelector("registration_time"),
        }
        this.status_button = this.identifiers_table.querySelector("#device-status-button");
        name_field = this.identifiers_table.querySelector("#name");

        this.name_field_content = name_field.querySelector("#name-input");
        this.name_field_content.value = device_name;
        this.name_field_error = name_field.querySelector("#error-text");
        this.name_field_button = name_field.querySelector("#change-name");

        unregister_button = document.getElementById("unregister-button");
        

        this.name_field_content.addEventListener('input', this.name_entry_field_event_handler);
        this.name_field_button.addEventListener('click', async function() {
            await this.change_name();
        }.bind(this));

        this.get_static_fields();
    }

    async get_static_fields() {
        route = format_route(`/devices/devices/${device_name}/device_info`);
        await fetch(route)
        .then(res => res.json())
        .then(json_response => {
            for(let field in json_response.keys()) {
                if(field in this.static_fields.keys()) {
                    this.static_fields[field].textContent = json_response[field];
                }
            }
        });
    }

    async get_status() {
        route = format_route(`/devices/devices/${device_name}/status`);

        await fetch(route)
        .then(res => res.json())
        .then(status => {
            if(!status) {
                this.status_button.textContent = "DISABLED";
                this.status_button.style.background = "RED";
            }
            else {
                this.status_button.textContent = "ACTIVE";
                this.status_button.style.background = "GREEN";
            }
        })
    }

    async change_name() {
        route = format_route(`/devices/devices/${device_name}/update`);
        await fetch(route, {
            method: "PUT",
            headers: {
                "Content-Type":"application/json"
            },
            body: JSON.stringify({
                device_name: this.name_field_content.value
            })
        })
        .then(res => res.json())
        .then(json_res => {
            alert("Device name successfully updated");
            document.getElementById("device_name").textContent = this.name_field_content.value;
        })
    }

    name_entry_field_event_handler() {
        if(this.name_field_content.value.length > 255) {
            this.name_field_error.textContent = "Name must be under 255 characters";
            this.name_field_button.style.display = "block";
        }
        else {
            this.name_field_error.textContent = "";
            this.name_field_button.style.display = "none";
        }
    }
}