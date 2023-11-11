import { asynchronous_updater, format_route } from "./shared/asynchronousUpdater.js";


class DeviceSelector {
    constructor() {
        this.source_project = document.getElementById("source_project").content;
        this.device_selection_table = document.getElementById("device-selector");
        this.device_selector_template = document.getElementById("device-selection-template");
        this.radio_buttons;
    }

    async get_scan() {
        var devices;
        var route = await format_route("/projects/scan/getscans")
        await fetch(route, {
            method: "GET",
        })
        .then(res => res.json())
        .then(data => {
            devices = data;
        })
        return devices
    }

    async display_scans() {
        var devices = await this.get_scan();
        for(let device in devices) {
            var device_radio = document.importNode(this.device_selector_template, true);
            var radio_button_text = device_radio.querySelector("#radio-button-label");
            var radio_button_data = device_radio.querySelector("#device-information");
            if(device['name']) {
                radio_button_text.textContent = device['name'];
            }
            else {
                radio_button_text.textContent = device['mac'];
            }
            radio_button_data.textContent = JSON.stringify(device);
            this.device_selection_table.append(device_radio);
        }
        this.radio_buttons = document.getElementsByName("radio-options");
    }

    async get_selected_device() {
        for(let button in this.radio_buttons) {
            if(button.checked) {
                var parent = button.parentNode;
                var device_data = parent.querySelector("#device-information").textContent;
                var device_data_parsed = JSON.parse(device_data);
                var re_formatted_data = {
                    device_ip: device_data_parsed.keys()[0],
                    device_name: device_data_parsed['name'],
                    device_mac: device_data_parsed['mac']
                }
                if(!re_formatted_data['device_namename']) {
                    re_formatted_data['device_name'] = re_formatted_data['device_mac']
                }

                var route = await format_route(`/devices/devices/register_device`)
                if(this.source_project) {
                    route = route.concat(`?project_name=${this.source_project}`);
                }
                await fetch(route,  
                    {
                        method: "POST",
                        headers: {
                            "Content-Type":"application/json"
                        },
                        body: JSON.stringify(re_formatted_data)
                    }
                )
                .then(res => res.status())
                .then(async function(rescode) {
                    if(rescode == 200) {
                        await this.device_page_redirect(re_formatted_data['device_name']);
                    }
                    else if(rescode == 503) {
                        alert(`Failed to register device`);
                    }
                }.bind(this))
                break;
            }
        }
    }

    async device_page_redirect(device_name) {
        document.location.href = await format_route(`/devices/devices/${device_name}`);
    }
}