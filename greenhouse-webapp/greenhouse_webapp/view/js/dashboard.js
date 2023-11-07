import { load_device, load_project, load_preset } from "./shared/objectRequest.js";

function get_data_from_request(request_content, data_needed) { // get specified datapoints to be displayed on buttons
    var data = "";
    if(data_needed.length == 1) {
        return request_content[data_needed[0]];
    }
    for(let index in data_needed) {
        data = data.concat(request_content[index]).concat(" | ")
    }
    return data.slice(0, -3)
}

async function load_request_handler(data_needed, load_request, template_id, table_id, request_path) {
    const table = document.getElementById(table_id);
    const template = document.getElementById(template_id);

    const response = await fetch(request_path)
    const request = await response.json()
    
    if(request.status === 200) {

        var response_content = request.responseText;
        var data = JSON.parse(response_content);

        for(let data_point in data) {
            load_request(get_data_from_request(data_point, data_needed), template, table);
        }
    }
    else {
        load_request("ERROR", template, table);
    }
}

async function format_route(route_path) {
    var port;
    var host;

    const response = await fetch("./config/controller.json")
    .then((res) => res.json())
    .then((config_data) => {
        port = config_data['port'];
        host = config_data['host'];
    });

    return `http://${host}:${port}${route_path}`
}

async function get_projects() {
    load_request_handler([1], load_project, "project-button", "projects_table", await format_route("/projects/list_projects"));
}

async function get_devices() {
    load_request_handler([1, 7], load_device, "device-button", "devices_table", await format_route("/devices/list_devices"));
}

async function get_presets() {
    load_request_handler([1], load_preset, "preset-button", "presets_table", await format_route("/presets/list_presets"));
}

async function dashboard_main() {
    while(true) {
        get_projects();
        get_devices();
        get_presets();
        await new Promise(function(resolve) {
            setTimeout(resolve, 10000)
        });
    }
}

dashboard_main()