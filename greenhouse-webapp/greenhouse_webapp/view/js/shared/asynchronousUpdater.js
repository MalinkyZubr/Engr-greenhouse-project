import { load_device, load_project, load_preset } from "./objectRequest.js";

function get_data_from_request(request_content, data_needed) { // get specified datapoints to be displayed on buttons
    var data = "";
    if(data_needed.length == 1) {
        return request_content[data_needed[0]];
    }
    var data = [];
    for(let x = 0; x < data_needed.length; x++) {
        data.push(request_content[data_needed[x]]);
    }
    return data
}

async function load_request_handler(data_needed, load_request, template_id, table_id, request_path) {
    const table = document.getElementById(table_id);
    console.log(table);
    console.log(table_id);
    const template = document.getElementById(template_id);
    table.innerHTML = "";

    const response = await fetch(request_path)
    const request = await response.json()
    
    if(response.status === 200) {
        for(let i = 0; i < request.length; i++) {
            load_request(get_data_from_request(request[i], data_needed), template, table);
        }
    }
    else {
        load_request("ERROR", template, table);
    }
}

async function format_route(route_path) {
    var port;
    var host;

    const response = await fetch("http://localhost:8000/config")
    .then((res) => res.json())
    .then((config_data) => {
        port = config_data['port'];
        host = config_data['host'];
    });
    console.log(host);
    console.log(port);

    console.log(`http://${host}:${port}${route_path}`)
    return `http://${host}:${port}${route_path}`
}

export async function get_projects() {
    load_request_handler([1], load_project, "project-button", "projects_table", await format_route("/projects/list_projects"));
}

export async function get_devices() {
    load_request_handler([1, 7], load_device, "device-button", "devices_table", await format_route("/devices/list_devices"));
}

export async function get_presets() {
    load_request_handler([1], load_preset, "preset-button", "presets_table", await format_route("/presets/list_presets"));
}

export async function get_project_associated_devices(project_name) {
    load_request_handler([1, 7], load_device, "device-button", "devices_table", (await format_route("/projects/projects")).concat(`/${project_name}/devices`))
}

export async function asynchronous_updater(asynchronous_functions) {
    while(true) {
        for(let i = 0; i < asynchronous_functions.length; i++) {
            await asynchronous_functions[i]();
        }
        await new Promise(function(resolve) {
            setTimeout(resolve, 10000)
        });
    }
}