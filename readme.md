# CCJ Gateway

The CCJ Gateway is an ESP32-based Wi-Fi-to-BLE gateway that allows users to operate a physical dormitory light switch through a web interface and remotely monitor the battery level of the switch unit.

## System Overview

The CCJ Gateway and ZZK Switch form a web-controlled smart light switch system developed for use in a student dormitory. The system uses a servo motor to physically operate an existing wall switch without modifying the room's electrical wiring.

The CCJ Gateway connects to a Wi-Fi network and hosts the browser-based control interface. When the user selects an ON or OFF command, the gateway receives the HTTP request and writes a one-byte command to the ZZK Switch through Bluetooth Low Energy (BLE).

The ZZK Switch periodically measures its battery voltage and sends the estimated battery percentage to the gateway through BLE notifications. The gateway then pushes the latest value to the web interface using Server-Sent Events (SSE).

This repository contains the software for the CCJ Gateway. The switch-side software is available in the [ZZK Switch repository](https://github.com/coauther/ZZK-BLE-Switch-Assistant).

## Responsibilities of the CCJ Gateway

The CCJ Gateway is responsible for:

- Connecting to Wi-Fi and hosting the web control interface
- Receiving ON and OFF commands through an HTTP endpoint
- Scanning for nearby ZZK Switch devices
- Binding to a selected switch using its BLE MAC address
- Storing the selected MAC address in non-volatile memory
- Transmitting one-byte control commands through BLE
- Receiving battery-level notifications from the ZZK Switch
- Pushing battery updates to the webpage through SSE
- Automatically scanning for the switch again after disconnection