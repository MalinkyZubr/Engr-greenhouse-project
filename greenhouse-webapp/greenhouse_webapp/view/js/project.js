import { asynchronous_updater, get_project_associated_devices } from "./shared/asynchronousUpdater.js"

let re = new RegExp("(?<=Project: ).*");

let project_name = re.exec(document.getElementById("project_name").textContent)[0];
console.log(project_name);

let functions = [
    async function() {
        get_project_associated_devices(project_name);
    }
];

asynchronous_updater(functions);