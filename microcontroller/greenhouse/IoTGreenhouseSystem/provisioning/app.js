let re = new RegExp('\\b(?:[0-9]{1,3}\\.){3}[0-9]{1,3}\\b')

class Manager {
    constructor() {
        this.radio_group = document.getElementsByName('network');
        this.username_div = document.getElementById('username_div');
        this.ssid_field = document.getElementById('ssid');
        this.username_field = document.getElementById('username');
        this.password_field = document.getElementById('password');
        this.network_type = 'home';
        this.device_ip = re.exec(window.location.href);

        submit_button = document.getElementById('submit');
        submit_button.addEventListener('click', this.submit_form);

        for(let x = 0; x < radio_group.length; x++) {
            this.radio_group[x].addEventListener('change', this.read_button);
        }
    }

    read_button() {
        for(let x = 0; x < radio_group.length; x++) {
            if(radio_group[x].checked) {
                if(radio_group[x].value === 'enterprise') {
                    username_field.style.display = 'block';
                    network_type = 'enterprise';
                }
                else if(radio_group[x].value === 'home') {
                    username_field.style.display = 'none';
                    network_type = 'home';
                }
            }
        }
    }

    confirmation_message() {
        var message_string = `SSID: ${ssid_field.value}`;
        if(network_type === 'enterprise') {
            message_string = message_string.concat(`\nUsername: ${username_field.value}`);
        }
        return message_string;
    }

    async submit_form() {
        var confirmation = confirm(`Confirm that this information is correct:\n${this.confirmation_message()}`);
        if(confirmation) {
            await fetch(this.device_ip + '/submit', {
                method: 'POST',
                headers: {
                    'Content-Type':'application/json'
                },
                body: JSON.stringify({
                    type: this.network_type,
                    ssid: this.ssid_field.value,
                    username: this.username_field.value,
                    password: this.password_field.value
                })
            });
            alert('The data has been submitted. If the connection to the selected Wifi network is successful, you will not be able to reconnect to the device!\nIf the connection fails, the device should re-activate its own wifi network. If your device doesnt automatically reconnect, you can reconnect in your wifi settings');
        }
        return;
    }
}
new Manager();