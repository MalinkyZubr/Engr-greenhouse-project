import { InstantiationList } from "./widgets/list/instantiationList.ts"
import { RequestButton } from "./widgets/buttons/requestButton.ts"
import { host } from "./config.ts"


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

class BaseDeviceSelector{};