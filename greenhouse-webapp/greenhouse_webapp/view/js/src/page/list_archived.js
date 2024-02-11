import { Assigner } from "./shared/assignment.js";
import { asynchronous_updater } from "../shared/asynchronousUpdater.js";


let assigner = new Assigner("archived");

let functions = [
    assigner.retrieve_elements
];

asynchronous_updater(functions);