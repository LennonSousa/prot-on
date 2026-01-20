# Prot-On — ESP8266 Automation Controller

Prot-On is a lightweight automation controller for ESP8266/Arduino boards. It hosts a small HTTP API and a static UI (LittleFS) to manage devices and time-based schedules.

This is a system to automate devices connected to a wireless network.
The system uses ESP-8266 or Arduino boards to run the server, accept requests and responses to an user.

## Features
- Device management (create/edit/delete/list)
- Schedule management (create/edit/delete/list)
- NTP time sync for scheduled actions
- JSON-based persistence on LittleFS

## HTTP API
- GET  /device — list devices
- POST /device — create device
- PUT  /device — edit device
- DELETE /device — delete device
- PUT  /device/status — change device status

- GET  /schedule — list schedules (filtered by deviceId)
- POST /schedule — create schedule
- PUT  /schedule — edit schedule
- DELETE /schedule — delete schedule

## Storage (LittleFS)
- /settings.json — Wi‑Fi and setup configuration
- /devices.json — registered devices
- /schedules.json — automation schedules

## Stack
**Front-end:** HTML, CSS, JavaScript

**Back-end:** C/C++, JSON
