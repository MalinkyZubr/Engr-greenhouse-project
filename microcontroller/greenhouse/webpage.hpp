#ifndef WEBPAGE_HPP
#define WEBPAGE_HPP


const PROGMEM char webpage_html[] = R"(<!doctypehtml><html lang=en><meta charset=UTF-8><meta content="width=device-width,initial-scale=1"name=viewport><title>Login Page</title><link href=/styles.css rel=stylesheet><script src=/app.js></script><div class=container><div class=header-item>Login</div><div class=itemA1><form><label for=ssid>SSID:</label> <input name=ssid class=textfield id=ssid placeholder="Enter your SSID"><div id=username_div><label for=username>Username:</label> <input name=username class=textfield id=username placeholder="Enter your username"></div><label for=password>Password:</label> <input name=password class=textfield id=password placeholder="Enter your password"type=password><div class=button id=submit>Login</div><div><label><input name=network type=radio value=enterprise> Enterprise</label> <label><input name=network type=radio value=home> Home</label></div></form></div></div>)";
const PROGMEM char webpage_css[] = R"(body { font-family: Arial, sans-serif; margin: 0; padding: 0; } .container { display: grid; grid-template-rows: 100px 300px; grid-template-columns: auto auto auto; padding: 20px; } .header-item { grid-column: span 3; grid-row: span 1; background-color: #444; color: #fff; text-align: center; padding: 10px; } .itemA1 { grid-column: span 3; grid-row: span 2; background-color: #f0f0f0; border: 1px solid #ccc; padding: 20px; } label { display: block; margin-bottom: 5px; } .textfield { width: 100%; padding: 10px; margin-bottom: 10px; box-sizing: border-box; } .button { background-color: green; color: #fff; text-align: center; padding: 10px; text-decoration: none; margin: 10px; width: 80%; cursor: pointer; } .button:hover { background-color: darkgreen; })";
const PROGMEM char webpage_js[] = R"(let re=RegExp("\\b(?:[0-9]{1,3}\\.){3}[0-9]{1,3}\\b");class Manager{constructor(){this.radio_group=document.getElementsByName("network"),this.username_div=document.getElementById("username_div"),this.ssid_field=document.getElementById("ssid"),this.username_field=document.getElementById("username"),this.password_field=document.getElementById("password"),this.network_type="home",this.device_ip=re.exec(window.location.href),(submit_button=document.getElementById("submit")).addEventListener("click",this.submit_form);for(let e=0;e<radio_group.length;e++)this.radio_group[e].addEventListener("change",this.read_button)}read_button(){for(let e=0;e<radio_group.length;e++)radio_group[e].checked&&("enterprise"===radio_group[e].value?(username_field.style.display="block",network_type="enterprise"):"home"===radio_group[e].value&&(username_field.style.display="none",network_type="home"))}confirmation_message(){var e=`SSID: ${ssid_field.value}`;return"enterprise"===network_type&&(e=e.concat(`
Username: ${username_field.value}`)),e}async submit_form(){confirm(`Confirm that this information is correct:
${this.confirmation_message()}`)&&(await fetch(this.device_ip+"/submit",{method:"POST",headers:{"Content-Type":"application/json"},body:JSON.stringify({type:this.network_type,ssid:this.ssid_field.value,username:this.username_field.value,password:this.password_field.value})}),alert("The data has been submitted. If the connection to the selected Wifi network is successful, you will not be able to reconnect to the device!\nIf the connection fails, the device should re-activate its own wifi network. If your device doesnt automatically reconnect, you can reconnect in your wifi settings"))}}new Manager;)"


#endif