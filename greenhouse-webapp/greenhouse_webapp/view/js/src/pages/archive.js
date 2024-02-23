import { ProjectManager } from "./project.js";


let project_manager = new ProjectManager(true);


let functions = [
    async function() {
        get_project_associated_devices(project_name);
    },
    async function() {
        project_manager.get_graph_image(project_name);
    }
];

asynchronous_updater(functions);