import { asynchronous_updater, get_devices, get_projects, get_presets } from "../shared/asynchronousUpdater.js";

asynchronous_updater([get_projects, get_devices, get_presets]);