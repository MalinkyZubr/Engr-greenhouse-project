export function load_project(project_name, project_template, project_table) {
    var project_button = document.importNode(project_template.children.contentEditable, true);
    var project_button_text = project_button.getElementById("project-button-text");
    project_button_text.textContent = project_name;

    project_button.addEventListener("click", project_redirect(project_name));

    project_table.append(project_button);
}

function project_redirect(project_name) {
    return function() { // must return an anonymous function for the eventListener
        window.location.href = "/projects/projects/".concat(project_name);
    }
}

export function load_preset(preset_name, preset_template, preset_table) {
    var preset_button = document.importNode(preset_template.children.contentEditable, true);
    var preset_button_text = preset_button.getElementById("preset-text");
    preset_button_text.textContent = preset_name;

    preset_button.addEventListener("click", preset_redirect(preset_name));

    preset_table.append(preset_button);
}

function preset_redirect(preset_name) {
    return function() {
        window.location.href = "/presets/presets/".concat(preset_name);
    }
}

export function load_device(device_info, device_template, device_table) { // device_info should be in format name | status
    var device_button = document.importNode(device_template.children.contentEditable, true);
    var device_text = device_button.getElementById("device-button-text");

    device_text.textContent = device_info;

    device_button.addEventListener("click", device_redirect(device_name));

    device_table.append(device_button);
}

function device_redirect(device_name) {
    return function() {
        window.location.href = "/devices/".concat(device_name);
    }
}