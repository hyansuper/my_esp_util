# Wifi Manager

## Usage

- Configure host name in menuconfig.

- `wmgr_init_host_name()` to set both host name and wifi ap ssid(no password) for wifi provision.

- `wmgr_prov()` to start wifi provision. then the user connects to wifi ap of esp32 device, usually a captive portal page will pop up, if not, visit http://<hostname>.local, fill in wifi ssid and password. You should restart device after successfully connected to wifi ap.

- After restart, call `wmgr_sta()` to start wifi sta mode and connect to previous selected wifi ap.

## Ref

follow the init sequence of wifi sta/ap mode as in
https://github.com/espressif/esp-idf/blob/master/examples/wifi/softap_sta/main/softap_sta.c

for captive portal, see
https://github.com/espressif/esp-idf/tree/master/examples/protocols/http_server/captive_portal