import { InstantiationList } from "./widgets/list/instantiationList.ts"
import { RequestButton } from "./widgets/buttons/requestButton.ts"
import { host } from "./config.ts"
import { PeriodicExecutor } from "./shared/periodic.ts";


class BaseDeviceSelectorList extends InstantiationList {
    public host: string = host;
    private route: string = "/devices/register_device"
    public widget_name: string = "DeviceSelector"

    public get_route(): string {
        return this.route;
    }
} 

class DeviceSelectorButton extends RequestButton {
    device_list: BaseDeviceSelectorList;

    constructor(widget_data: object, parent_element: HTMLElement, widget_key: string | undefined, device_list: BaseDeviceSelectorList) {
        super(widget_data, parent_element, widget_key)
        this.device_list = device_list;
    }

    public async submit_button_request(): Promise<void> {
        await fetch(`http://${this.device_list.host}${this.device_list.get_route()}`, {
            method: "POST",
            headers: {
                "Content-Type":"application/json"
            },
            body: JSON.stringify(this.device_list.get_element_by_id(this.device_list.get_selected_value()))
        })
        .then(res => res.status)
        .then(function(rescode: number) {
                if(rescode === 200) {
                    this.device_page_redirect();
                }
                else {
                    alert("failed to register device");
                }
            }.bind(this)
        );
        return;
    }

    private device_page_redirect(device_name: string): void {
        document.location.href = `http://${this.device_list.host}/devices/devices/${device_name}`;
    }
};

class BaseDeviceSelector extends PeriodicExecutor{
    private submit_button: DeviceSelectorButton;
    private device_list: BaseDeviceSelectorList;

    constructor() {
        super();

        var list_container: HTMLElement = document.getElementById("list_container") ?? function() {throw new Error("List container not found")}();
        var button_container: HTMLElement = document.getElementById("button_container") ?? function() {throw new Error("Button container not found")}();

        this.device_list = new BaseDeviceSelectorList(
            {
                element_id: "selector_list",
                list_name: "Devices Ready To Register",
                table_header: "Device",
            },
            list_container,
            "scan_list"

        )
        this.submit_button = new DeviceSelectorButton(
            {
                element_id: "sumbit_button", 
                button_name: "Register Device"
            }, 
            button_container, 
            "register_button",
            this.device_list
        );
    }

    async call_methods(): Promise<void> {
        await this.device_list.update_instantiation_list();
    }
};