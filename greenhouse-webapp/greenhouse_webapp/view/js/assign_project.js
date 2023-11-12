import { asynchronous_updater } from "./shared/asynchronousUpdater.js";
import { Assigner } from "./shared/assignment.js";


let assigner = new Assigner("project_name");

let functions = [
    assigner.retrieve_elements
];

asynchronous_updater(functions);