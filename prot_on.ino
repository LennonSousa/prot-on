/*
  Prot-On — ESP8266 Automation Controller

  Overview
  - Hosts an HTTP API and static UI (LittleFS) for device management.
  - Manages devices and schedules stored in JSON files.
  - Uses NTP to keep time for scheduled actions.

  Key Endpoints
  - GET  /device            List devices
  - POST /device            Create device
  - PUT  /device            Edit device
  - DELETE /device          Delete device
  - PUT  /device/status     Change device status
  - GET  /schedule          List schedules (filtered by deviceId)
  - POST /schedule          Create schedule
  - PUT  /schedule          Edit schedule
  - DELETE /schedule        Delete schedule

  Files (LittleFS)
  - /settings.json  Wi-Fi and setup configuration
  - /devices.json   Registered devices
  - /schedules.json Automation schedules
*/

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <LittleFS.h> //Include File System Headers
#include <SNTPtime.h>
#include <ArduinoJson.h>
#include <UUID.h>

// define números de pinos
const int localPort = 4; // GPIO4
int devicesFound;
int schedulesFound;
const char *imagefile = "/image.png";
const char *htmlfile = "/index.html";

JsonDocument wifiScanResults;
JsonDocument settingsJSONResults;
JsonDocument devicesJSONResults;
JsonDocument schedulesJSONResults;

#define SettingsFile "/settings.json"
#define DevicesFile "/devices.json"
#define SchedulesFile "/schedules.json"
#define WifiScanResutsFile "/wifi_scan_results.json"

#define MAX_CONNECTION_ATTEMPTS 30
#define MAX_QTD_DISP 32
#define MAX_QTD_ALARMES 64
#define MAX_SSID_AMOUNT 10

typedef struct
{
  bool configured;
  String ssid;
  String password;
} Setting;

typedef struct
{
  String id;
  String name;
  String ip;
  bool main;
} Device;

typedef struct
{
  String id;
  String name;
  String deviceId;
  String hour;
  String minute;
  String action;
  bool active;
} Schedule;

typedef struct
{
  UUID id;
  String ssid;
  bool secure;
} SSID;

Setting settings;

// Last time (millis) when we checked alarms to avoid checking every loop
unsigned long lastAlarmCheckMillis = 0;

ESP8266WebServer server(80);

char ntpServer[] = "br.pool.ntp.org";
SNTPtime NTPch(ntpServer);
strDateTime dateTime;
// Enable to print parsed JSON for debugging
const bool DEBUG_JSON = false;

// Helpers to load/save the in-memory documents
void loadDevicesFromFile()
{
  getJSONFromFile(&devicesJSONResults, DevicesFile);
  JsonArray arr = devicesJSONResults.as<JsonArray>();
  devicesFound = arr.size();
}

void saveDevicesToFile()
{
  saveJsonToAFile(&devicesJSONResults, DevicesFile);
  JsonArray arr = devicesJSONResults.as<JsonArray>();
  devicesFound = arr.size();
}

void loadSchedulesFromFile()
{
  getJSONFromFile(&schedulesJSONResults, SchedulesFile);
  JsonArray arr = schedulesJSONResults.as<JsonArray>();
  schedulesFound = arr.size();
}

void saveSchedulesToFile()
{
  saveJsonToAFile(&schedulesJSONResults, SchedulesFile);
  JsonArray arr = schedulesJSONResults.as<JsonArray>();
  schedulesFound = arr.size();
}

JsonDocument getJSONFromFile(JsonDocument *doc, String fileName)
{
  // open the file for reading:
  String text = loadFile(fileName);

  if (text)
  {
    DeserializationError error = deserializeJson(*doc, text);
    if (error)
    {
      // if the file didn't open, print an error:
      if (DEBUG_JSON)
      {
        Serial.print(F("Error parsing JSON "));
        Serial.println(error.c_str());
      }

      return doc->to<JsonObject>();
    }

    if (DEBUG_JSON)
    {
      serializeJson(*doc, Serial);
      Serial.println();
    }

    return *doc;
  }
  else
  {
    if (DEBUG_JSON)
    {
      Serial.print(F("Error opening (or file not exists) "));
      Serial.println(fileName);

      Serial.println(F("Empty json returned"));
    }

    return doc->to<JsonObject>();
  }
}

Setting jsonToSettings(JsonDocument &doc)
{
  Setting settingsResponse;

  settingsResponse.configured = doc["configured"] | false;
  settingsResponse.ssid = doc["ssid"] | "";
  settingsResponse.password = doc["password"] | "";

  return settingsResponse;
}

Device jsonToDevice(JsonDocument &obj)
{
  Device deviceResponse;

  deviceResponse.id = String(obj["id"] | "");
  deviceResponse.name = obj["name"] | "";
  deviceResponse.ip = obj["ip"] | "";
  deviceResponse.main = obj["main"] | false;

  return deviceResponse;
}

Schedule jsonToSchedule(JsonVariantConst obj)
{
  Schedule scheduleResponse;

  scheduleResponse.id = String(obj["id"] | "");
  scheduleResponse.name = String(obj["name"] | "");
  scheduleResponse.deviceId = String(obj["deviceId"] | "");
  scheduleResponse.hour = String(obj["hour"] | "");
  scheduleResponse.minute = String(obj["minute"] | "");
  scheduleResponse.action = String(obj["action"] | "");
  scheduleResponse.active = obj["active"] | true;

  return scheduleResponse;
}

void deviceToJSON(const Device &device, JsonDocument &obj)
{
  obj["id"] = device.id;
  obj["name"] = device.name;
  obj["ip"] = device.ip;
  obj["main"] = device.main;
}

// Returns index of the device with given id in the provided JsonArray, or -1 if not found
int findDeviceIndexById(JsonArray devicesJSONArray, const String &id)
{
  for (size_t i = 0; i < devicesJSONArray.size(); ++i)
  {
    JsonObject dev = devicesJSONArray[i].as<JsonObject>();
    String did = String(dev["id"] | "");
    if (did == id)
      return (int)i;
  }
  return -1;
}

void setupWiFiGotIPHandler()
{
  // Espera enquanto não conecta ao roteador
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < MAX_CONNECTION_ATTEMPTS)
  {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  // Configurações da rede
  // IPAddress ip;
  // IPAddress gateway;
  // IPAddress subnet;
  // ip.fromString(IP);
  // gateway.fromString(GATEWAY);
  // subnet.fromString(SUBNET);

  // Serial.println(ip);

  // Envia para o roteador as configurações que queremos para o ESP
  // WiFi.config(ip, gateway, subnet);
  if (attempts < MAX_CONNECTION_ATTEMPTS)
  {
    Serial.println(WiFi.localIP());
    Serial.println("");
    Serial.println("Conectado");

    while (!NTPch.setSNTPtime())
    {
      Serial.print(".");
    }

    Serial.println();
    Serial.println("Hora ajustada");
  }
  else
  {
    Serial.println("Sem conexao");
  }
}

void setup()
{
  delay(1000);
  Serial.begin(9600);

  // Initialize File System
  if (!LittleFS.begin())
  {
    Serial.println("An Error has occurred while mounting LittleFS");
    return;
  }

  // Load settings, devices and schedules into memory once
  getJSONFromFile(&settingsJSONResults, SettingsFile);
  loadDevicesFromFile();
  loadSchedulesFromFile();

  settings = jsonToSettings(settingsJSONResults);

  pinMode(localPort, OUTPUT);

  // Inicia o ponto de acesso se está configurado
  if (settings.configured)
  {
    // Muda a configuração para estação
    WiFi.mode(WIFI_STA);
    Serial.println("Configurado como estacao");

    Serial.print("ssid = ");
    Serial.println(settings.ssid);

    Serial.print("senha = ");
    Serial.println(settings.password);

    // Conecta o ESP ao nosso roteador
    WiFi.begin(settings.ssid, settings.password);

    Serial.println("Conectando (async)");

    setupWiFiGotIPHandler();
  }
  else
  {
    // Muda a configuração para estação e ponto de acesso
    String mac = WiFi.softAPmacAddress();
    String lastDigits = mac.substring(mac.length() - 5);
    lastDigits.replace(":", "");
    String ssid = "Prot-On-" + lastDigits;

    WiFi.mode(WIFI_AP_STA);
    Serial.println("Configurando como soft-AP ... ");
    Serial.println(WiFi.softAP(ssid) ? "Ready" : "Failed!");
  }

  // Initialize Webserver
  server.on("/", HTTP_GET, inicio);

  // First Settings
  server.on("/first-setting/device", HTTP_PUT, editDevice);
  server.on("/first-setting/finish", HTTP_POST, finalizaConfig);

  // Wireless
  server.on("/wireless", HTTP_GET, searchWireless);
  server.on("/wireless", HTTP_POST, connectWireless);

  // Device
  server.on("/device", HTTP_POST, createDevice);
  server.on("/device", HTTP_GET, listDevices);
  server.on("/device", HTTP_PUT, editDevice);
  server.on("/device", HTTP_DELETE, deleteDevice);
  server.on("/device/status", HTTP_PUT, changeDeviceStatus);

  // Schedules
  server.on("/schedule", HTTP_POST, createSchedule);
  server.on("/schedule", HTTP_GET, listSchedules);
  server.on("/schedule", HTTP_PUT, editSchedule);
  server.on("/schedule", HTTP_DELETE, deleteSchedule);

  server.onNotFound(handleWebRequests); // Set setver all paths are not found so we can handle as per URI

  server.begin();
}

void loop()
{
  server.handleClient();

  unsigned long now = millis();
  // Check alarms at most once per minute (60000 ms)
  if (now - lastAlarmCheckMillis >= 60001UL)
  {
    lastAlarmCheckMillis = now;

    dateTime = NTPch.getTime(-3, 0); // get/refresh time once per minute
    byte currentHour = dateTime.hour;
    byte currentMinute = dateTime.minute;

    Serial.print("Checking alarms at: ");
    Serial.print(currentHour);
    Serial.print(":");
    Serial.println(currentMinute);

    // Call alarm checker with current time
    checkForAlarm(currentHour, currentMinute);
  }
}

void inicio()
{
  Serial.println("Entrou no inicio");

  if (settings.configured)
  {
    Serial.println("Ja configurado");

    server.sendHeader("Location", "/index.html", true); // Redirect to our html web page
  }
  else
  {
    Serial.println("Ainda nao configurado");
    server.sendHeader("Location", "/primeirospassos.html", true); // Redirect to our html web page
  }

  server.send(302, "text/plain", "");
}

void listDevices()
{
  JsonArray devicesJSONArray = devicesJSONResults.as<JsonArray>();

  for (int i = 0; i < devicesFound; i++)
  {
    JsonDocument deviceObj = devicesJSONArray[i];
    Device device = jsonToDevice(deviceObj);

    if (device.main)
    {
      devicesJSONArray[i]["status"] = digitalRead(localPort) == LOW ? "0" : "1";
    }
    else
    {
      devicesJSONArray[i]["status"] = "error"; // TODO: Default status for non-main devices
    }
  }

  if (DEBUG_JSON)
  {
    serializeJson(devicesJSONArray, Serial);
    Serial.println();
  }

  const String response = [&]()
  {
    String out;
    serializeJson(devicesJSONArray, out);
    return out;
  }();

  server.send(200, "application/json", response);
}

void listSchedules()
{
  if (!server.hasArg("deviceId"))
  {
    return BadRequestError("deviceId is required");
  }
  String filterDeviceId = server.arg("deviceId");

  JsonArray schedulesJSONArray = schedulesJSONResults.as<JsonArray>();
  JsonArray devicesJSONArray = devicesJSONResults.as<JsonArray>();

  schedulesFound = schedulesJSONArray.size();

  DynamicJsonDocument outDoc(2048);
  JsonArray outArr = outDoc.to<JsonArray>();

  for (size_t i = 0; i < schedulesJSONArray.size(); ++i)
  {
    JsonObject scheduleObj = schedulesJSONArray[i].as<JsonObject>();

    String deviceId = String(scheduleObj["deviceId"] | "");
    if (deviceId != filterDeviceId)
      continue;

    bool active = scheduleObj.containsKey("active") ? (bool)scheduleObj["active"] : true;
    if (!active)
      continue;

    String deviceName = "unknown";
    int deviceIdx = findDeviceIndexById(devicesJSONArray, deviceId);
    if (deviceIdx >= 0)
    {
      JsonObject deviceObj = devicesJSONArray[deviceIdx].as<JsonObject>();
      deviceName = String(deviceObj["name"] | "");
    }

    JsonObject copied = outArr.createNestedObject();
    for (JsonPair kv : scheduleObj)
    {
      copied[kv.key()] = kv.value();
    }

    String hour = String(scheduleObj["hour"] | "");
    String minute = String(scheduleObj["minute"] | "");
    if (hour.length() == 1)
      hour = "0" + hour;
    if (minute.length() == 1)
      minute = "0" + minute;

    copied["deviceName"] = deviceName;
    copied["time"] = hour + ":" + minute;
  }

  if (DEBUG_JSON)
  {
    serializeJson(outArr, Serial);
    Serial.println();
  }

  const String response = [&]()
  {
    String out;
    serializeJson(outArr, out);
    return out;
  }();

  server.send(200, "application/json", response);
}

void changeDeviceStatus()
{
  if (!server.hasArg("id") || !server.hasArg("status"))
  {
    return BadRequestError("Id and status are required");
  }

  String id = server.arg("id");
  String newStatus = server.arg("status");

  JsonDocument jsonResponse;
  JsonArray devicesJSONArray = devicesJSONResults.as<JsonArray>();

  int idx = findDeviceIndexById(devicesJSONArray, id);
  if (idx < 0)
  {
    return BadRequestError("Device not found");
  }

  JsonDocument deviceObj = devicesJSONArray[idx];
  Device device = jsonToDevice(deviceObj);

  if (device.main)
  {
    if (newStatus == "0")
    {
      digitalWrite(localPort, LOW); // LED ON
      deviceObj["status"] = "0";    // Feedback parameter
    }
    else
    {
      digitalWrite(localPort, HIGH); // LED OFF
      deviceObj["status"] = "1";     // Feedback parameter
    }

    jsonResponse = deviceObj;
  }
  else
  {
    deviceObj["status"] = "error"; // TODO: Default feedback for non-main devices
    jsonResponse = deviceObj;
  }

  if (DEBUG_JSON)
  {
    serializeJson(jsonResponse, Serial);
    Serial.println();
  }

  const String response = [&]()
  {
    String out;
    serializeJson(jsonResponse, out);
    return out;
  }();

  server.send(200, "application/json", response);
}

bool saveJsonToAFile(JsonDocument *doc, String fileName)
{
  if (DEBUG_JSON)
  {
    Serial.print(F("Start write..."));
  }

  String jsonString = "";

  serializeJson(*doc, jsonString);

  if (DEBUG_JSON)
  {
    Serial.printf("Json to save");
    Serial.println(jsonString);
  }

  salvaArquivo(jsonString, fileName, false);

  if (DEBUG_JSON)
  {
    Serial.print(F("..."));
    // close the file:
    Serial.println(F("done."));
  }

  return true;
}

void searchWireless()
{
  JsonDocument obj = getJSONFromFile(&wifiScanResults, WifiScanResutsFile);

  JsonArray results;
  // Check if exist the array
  if (!obj.containsKey(F("results")))
  {
    Serial.println(F("Not find results array! Crete one!"));

    results = obj.createNestedArray(F("results"));

    WiFi.scanNetworksAsync(printScanResult);

    server.send(204, "application/json");
  }
  else
  {
    Serial.println(F("Find results array!"));
    results = obj[F("results")];

    String response = loadFile(WifiScanResutsFile);

    server.send(200, "application/json", response);
  }
}

void printScanResult(int networksFound)
{
  Serial.printf("%d network(s) found\n", networksFound);
  for (int i = 0; i < networksFound; i++)
  {
    Serial.printf("%d: %s, %s\n", i + 1, WiFi.SSID(i).c_str(), WiFi.encryptionType(i) == ENC_TYPE_NONE ? "aberto" : "seguro");
  }

  // Use ArduinoJson for safe JSON building
  JsonDocument doc; // Adjust size based on expected networks
  doc["count"] = networksFound;

  JsonArray resultsArray = doc.createNestedArray("results");
  for (int i = 0; i < networksFound; i++)
  {
    JsonObject network = resultsArray.createNestedObject();
    network["id"] = String(i + 1);
    network["ssid"] = WiFi.SSID(i);
    network["secure"] = (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? false : true;
  }

  saveJsonToAFile(&doc, WifiScanResutsFile);

  String jsonString;
  serializeJson(doc, jsonString);
}

void connectWireless()
{
  if (!server.hasArg("plain"))
  {
    return BadRequestError("Body is required");
  }

  String body = server.arg("plain");
  JsonDocument jsonBody;

  DeserializationError error = deserializeJson(jsonBody, body);
  if (error)
  {
    return BadRequestError("Invalid JSON");
  }

  const String networkSSID = jsonBody["ssid"] | "";
  const String networkPassword = jsonBody["password"] | "";

  char ssid[32];
  char password[64];

  networkSSID.toCharArray(ssid, 32);
  networkPassword.toCharArray(password, 64);

  // Conecta o ESP ao nosso roteador
  WiFi.begin(ssid, password);
  Serial.println("Conectando (async)");

  Serial.println("");
  Serial.print("Conectando");

  // Espera enquanto não conecta ao roteador
  String baseUrl = "";

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 15)
  {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (attempts < 15)
  {
    baseUrl = "http://" + WiFi.localIP().toString();
    Serial.print("Novo IP: ");
    Serial.println(WiFi.localIP());
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    JsonDocument settingsJSON = getJSONFromFile(&settingsJSONResults, SettingsFile);

    settingsJSON["ssid"] = networkSSID;
    settingsJSON["password"] = networkPassword;

    saveJsonToAFile(&settingsJSON, SettingsFile);

    JsonArray devicesJSONArray = devicesJSONResults.as<JsonArray>();

    for (int i = 0; i < devicesFound; i++)
    {
      JsonDocument deviceObj = devicesJSONArray[i];
      Device device = jsonToDevice(deviceObj);

      if (device.main)
      {
        UUID uuid;
        devicesJSONArray[i]["id"] = uuid.toCharArray();
        devicesJSONArray[i]["ip"] = WiFi.localIP().toString();

        saveDevicesToFile();

        break;
      }
    }
  }

  WiFi.printDiag(Serial);
  Serial.println(baseUrl);

  JsonDocument jsonResponse;
  jsonResponse["baseUrl"] = baseUrl;

  String response;
  serializeJson(jsonResponse, response);
  server.send(201, "application/json", response);
}

void finalizaConfig()
{
  JsonDocument settingsJSON = getJSONFromFile(&settingsJSONResults, SettingsFile);

  settingsJSON["configured"] = true;

  saveJsonToAFile(&settingsJSON, SettingsFile);

  server.send(201, "application/json");
  delay(1000);

  ESP.restart();
}

String loadFile(String caminho)
{
  File arquivo = LittleFS.open(caminho, "r");
  String response = "";

  // Se o arquivo existe
  if (arquivo)
  {

    while (arquivo.available())
    {
      response += (char)arquivo.read();
    }
    // close the file:
    arquivo.close();

    if (DEBUG_JSON)
      Serial.println(response);

    return response;
  }
  else
  {
    Serial.println("Error to load file");
    Serial.println(arquivo);

    return response;
  }
}

bool salvaArquivo(String textoNovo, String caminho, bool recarrega)
{
  File arquivo = LittleFS.open(caminho, "w");
  String configuracoes = "";

  // Se o arquivo existe
  if (arquivo)
  {
    // Escreve o novo texto no arquivo sem null terminator
    arquivo.print(textoNovo);

    // Fecha o arquivo:
    arquivo.close();

    return true;
  }
  else
  {
    Serial.println("Erro ao salvar o arquivo");
    return false;
  }
}

bool loadFromSpiffs(String path)
{
  String dataType = "text/plain";
  if (path.endsWith("/"))
    path += "index.htm";

  if (path.endsWith(".src"))
    path = path.substring(0, path.lastIndexOf("."));
  else if (path.endsWith(".html"))
    dataType = "text/html";
  else if (path.endsWith(".htm"))
    dataType = "text/html";
  else if (path.endsWith(".css"))
    dataType = "text/css";
  else if (path.endsWith(".js"))
    dataType = "application/javascript";
  else if (path.endsWith(".png"))
    dataType = "image/png";
  else if (path.endsWith(".gif"))
    dataType = "image/gif";
  else if (path.endsWith(".jpg"))
    dataType = "image/jpeg";
  else if (path.endsWith(".ico"))
    dataType = "image/x-icon";
  else if (path.endsWith(".xml"))
    dataType = "text/xml";
  else if (path.endsWith(".pdf"))
    dataType = "application/pdf";
  else if (path.endsWith(".zip"))
    dataType = "application/zip";
  File dataFile = LittleFS.open(path.c_str(), "r");

  if (server.hasArg("download"))
    dataType = "application/octet-stream";

  if (server.streamFile(dataFile, dataType) != dataFile.size())
  {
  }

  dataFile.close();
  return true;
}

void createDevice()
{
  if (devicesFound >= MAX_QTD_DISP)
  {
    return BadRequestError("Maximum number of devices reached");
  }

  if (!server.hasArg("plain"))
  {
    return BadRequestError("Body is required");
  }

  String body = server.arg("plain");
  JsonDocument doc;

  DeserializationError error = deserializeJson(doc, body);
  if (error)
  {
    return BadRequestError("Invalid JSON");
  }

  const String name = doc["name"];
  const String ip = doc["ip"];

  const bool nameProvided = doc.containsKey("name") && !doc["name"].isNull() && name.length() > 0;
  const bool ipProvided = doc.containsKey("ip") && !doc["ip"].isNull() && ip.length() > 0;

  if (!nameProvided || !ipProvided)
  {
    return BadRequestError("Name and IP are required");
  }

  IPAddress ipAddr;
  if (!ipAddr.fromString(ip))
  {
    return BadRequestError("Invalid IP format");
  }

  JsonDocument jsonResponse;
  JsonArray devicesJSONArray = devicesJSONResults.as<JsonArray>();

  UUID uuid;
  JsonDocument newDeviceObj;

  newDeviceObj["id"] = uuid.toCharArray();
  newDeviceObj["name"] = name;
  newDeviceObj["ip"] = ip;
  newDeviceObj["main"] = false;

  devicesJSONArray.add(newDeviceObj);

  devicesFound++;

  saveDevicesToFile();
  deviceToJSON(jsonToDevice(newDeviceObj), jsonResponse);

  if (DEBUG_JSON)
  {
    serializeJson(jsonResponse, Serial);
    Serial.println();
  }

  const String response = [&]()
  {
    String out;
    serializeJson(jsonResponse, out);
    return out;
  }();

  server.send(201, "application/json", response);
}

void editDevice()
{
  if (settings.configured && !server.hasArg("id"))
  {
    return BadRequestError("Id is required");
  }

  if (!server.hasArg("plain"))
  {
    return BadRequestError("Body is required");
  }

  String body = server.arg("plain");
  JsonDocument doc;

  DeserializationError error = deserializeJson(doc, body);
  if (error)
  {
    return BadRequestError("Invalid JSON");
  }

  String id = server.arg("id");
  const String name = doc["name"];
  const String ip = doc["ip"];

  const bool nameProvided = doc.containsKey("name") && !doc["name"].isNull() && name.length() > 0;
  const bool ipProvided = doc.containsKey("ip") && !doc["ip"].isNull() && ip.length() > 0;

  if (!nameProvided)
  {
    return BadRequestError("Name is required");
  }

  IPAddress ipAddr;
  if (ipProvided && !ipAddr.fromString(ip))
  {
    return BadRequestError("Invalid IP format");
  }

  bool found = false;
  JsonDocument jsonResponse;
  JsonArray devicesJSONArray = devicesJSONResults.as<JsonArray>();

  // When device isn't configured yet, edit the main device
  if (!settings.configured)
  {
    JsonArray devicesJSONArray = devicesJSONResults.as<JsonArray>();

    for (int i = 0; i < devicesFound; i++)
    {
      JsonDocument deviceObj = devicesJSONArray[i];
      Device device = jsonToDevice(deviceObj);
      if (device.main)
      {
        devicesJSONArray[i]["name"] = name;
        device.name = name;

        saveDevicesToFile();
        deviceToJSON(device, jsonResponse);
        found = true;
        break;
      }
    }
  }
  else
  {
    int idx = findDeviceIndexById(devicesJSONArray, id);
    if (idx >= 0)
    {
      JsonDocument deviceObj = devicesJSONArray[idx];
      Device device = jsonToDevice(deviceObj);

      if (nameProvided)
      {
        devicesJSONArray[idx]["name"] = name;
        device.name = name;
      }

      if (ipProvided && !device.main)
      {
        devicesJSONArray[idx]["ip"] = ip;
        device.ip = ip;
      }

      saveDevicesToFile();
      deviceToJSON(device, jsonResponse);
      found = true;
    }
  }

  if (!found)
  {
    return BadRequestError("Device not found");
  }

  if (DEBUG_JSON)
  {
    serializeJson(jsonResponse, Serial);
    Serial.println();
  }

  const String response = [&]()
  {
    String out;
    serializeJson(jsonResponse, out);
    return out;
  }();

  server.send(200, "application/json", response);
}

void deleteDevice()
{
  if (!server.hasArg("id"))
  {
    return BadRequestError("Id is required");
  }

  String id = server.arg("id");
  bool found = false;
  JsonArray devicesJSONArray = devicesJSONResults.as<JsonArray>();

  int idx = findDeviceIndexById(devicesJSONArray, id);
  if (idx >= 0)
  {
    JsonDocument deviceObj = devicesJSONArray[idx];
    Device device = jsonToDevice(deviceObj);

    if (!device.main)
    {
      devicesJSONArray.remove(idx);
      devicesFound--;

      saveDevicesToFile();
      found = true;
    }
  }

  if (found)
  {
    server.send(204, "application/json");
  }
  else
  {
    return BadRequestError("Device not found or is main device");
  }
}

bool validateScheduleFields(const String &deviceId, const String &hour, const String &minute, bool checkDeviceExists, String &outError)
{
  auto isDigits = [](const String &s)
  {
    if (s.length() == 0)
      return false;
    for (size_t i = 0; i < s.length(); ++i)
      if (!isDigit(s[i]))
        return false;
    return true;
  };

  if (!isDigits(hour) || !isDigits(minute))
  {
    outError = "Hour and minute must be numeric";
    return false;
  }

  int h = hour.toInt();
  int m = minute.toInt();
  if (h < 0 || h > 23 || m < 0 || m > 59)
  {
    outError = "Hour must be 0-23 and minute 0-59";
    return false;
  }

  if (checkDeviceExists)
  {
    JsonArray devicesJSONArray = devicesJSONResults.as<JsonArray>();
    int idx = findDeviceIndexById(devicesJSONArray, deviceId);
    if (idx < 0)
    {
      outError = "deviceId not found";
      return false;
    }
  }

  return true;
}

void createSchedule()
{
  if (!server.hasArg("plain"))
  {
    return BadRequestError("Body is required");
  }

  String body = server.arg("plain");
  JsonDocument doc;

  DeserializationError error = deserializeJson(doc, body);
  if (error)
  {
    return BadRequestError("Invalid JSON");
  }

  const String name = doc["name"] | "";
  const String deviceId = doc["deviceId"] | "";
  const String hour = doc["hour"] | "";
  const String minute = doc["minute"] | "";
  const String action = doc["action"] | "";
  const bool active = doc.containsKey("active") ? (bool)doc["active"] : true;

  const bool nameProvided = name.length() > 0;
  const bool deviceProvided = deviceId.length() > 0;
  const bool hourProvided = hour.length() > 0;
  const bool minuteProvided = minute.length() > 0;

  if (!nameProvided || !deviceProvided || !hourProvided || !minuteProvided)
  {
    return BadRequestError("Name, deviceId, hour and minute are required");
  }

  JsonArray schedulesJSONArray = schedulesJSONResults.as<JsonArray>();

  auto isDigits = [](const String &s)
  {
    if (s.length() == 0)
      return false;
    for (size_t i = 0; i < s.length(); ++i)
      if (!isDigit(s[i]))
        return false;
    return true;
  };

  if (!isDigits(hour) || !isDigits(minute))
  {
    return BadRequestError("Hour and minute must be numeric");
  }

  int h = hour.toInt();
  int m = minute.toInt();

  if (h < 0 || h > 23 || m < 0 || m > 59)
  {
    return BadRequestError("Hour must be 0-23 and minute 0-59");
  }

  JsonArray devicesJSONArray = devicesJSONResults.as<JsonArray>();

  int idx = findDeviceIndexById(devicesJSONArray, deviceId);
  if (idx < 0)
  {
    return BadRequestError("deviceId not found");
  }
  String validationError;
  if (!validateScheduleFields(deviceId, hour, minute, true, validationError))
  {
    return BadRequestError(validationError);
  }

  UUID uuid;
  JsonDocument newScheduleObj;

  newScheduleObj["id"] = uuid.toCharArray();
  newScheduleObj["name"] = name;
  newScheduleObj["deviceId"] = deviceId;
  newScheduleObj["hour"] = hour;
  newScheduleObj["minute"] = minute;
  newScheduleObj["action"] = action;
  newScheduleObj["active"] = active;

  schedulesJSONArray.add(newScheduleObj);

  schedulesFound = schedulesJSONArray.size();

  saveSchedulesToFile();

  JsonDocument jsonResponse;
  jsonResponse = newScheduleObj;

  if (DEBUG_JSON)
  {
    serializeJson(jsonResponse, Serial);
    Serial.println();
  }

  const String response = [&]()
  {
    String out;
    serializeJson(jsonResponse, out);
    return out;
  }();

  server.send(201, "application/json", response);
}

void editSchedule()
{
  if (!server.hasArg("id"))
  {
    return BadRequestError("Id is required");
  }

  if (!server.hasArg("plain"))
  {
    return BadRequestError("Body is required");
  }

  String id = server.arg("id");
  String body = server.arg("plain");

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, body);
  if (error)
  {
    return BadRequestError("Invalid JSON");
  }

  JsonArray schedulesJSONArray = schedulesJSONResults.as<JsonArray>();

  bool found = false;
  JsonDocument jsonResponse;

  for (int i = 0; i < (int)schedulesJSONArray.size(); i++)
  {
    JsonDocument scheduleObj = schedulesJSONArray[i];
    String sid = String(scheduleObj["id"] | "");

    if (sid == id)
    {
      if (doc.containsKey("name"))
        schedulesJSONArray[i]["name"] = doc["name"];

      if (doc.containsKey("deviceId"))
      {
        String newDeviceId = String(doc["deviceId"] | "");

        JsonArray devicesJSONArray = devicesJSONResults.as<JsonArray>();
        int deviceIdx = findDeviceIndexById(devicesJSONArray, newDeviceId);
        if (deviceIdx < 0)
          return BadRequestError("deviceId not found");

        schedulesJSONArray[i]["deviceId"] = doc["deviceId"];
      }

      if (doc.containsKey("hour") || doc.containsKey("minute"))
      {
        String newHour = doc.containsKey("hour") ? String(doc["hour"] | "") : String(schedulesJSONArray[i]["hour"] | "");
        String newMinute = doc.containsKey("minute") ? String(doc["minute"] | "") : String(schedulesJSONArray[i]["minute"] | "");

        String validationError;
        if (!validateScheduleFields("", newHour, newMinute, false, validationError))
        {
          return BadRequestError(validationError);
        }

        schedulesJSONArray[i]["hour"] = newHour;
        schedulesJSONArray[i]["minute"] = newMinute;
      }

      if (doc.containsKey("action"))
        schedulesJSONArray[i]["action"] = doc["action"];

      if (doc.containsKey("active"))
        schedulesJSONArray[i]["active"] = doc["active"];

      saveSchedulesToFile();

      jsonResponse = schedulesJSONArray[i];
      found = true;
      break;
    }
  }

  if (!found)
  {
    return BadRequestError("Schedule not found");
  }

  if (DEBUG_JSON)
  {
    serializeJson(jsonResponse, Serial);
    Serial.println();
  }

  const String response = [&]()
  {
    String out;
    serializeJson(jsonResponse, out);
    return out;
  }();

  server.send(200, "application/json", response);
}

void deleteSchedule()
{
  if (!server.hasArg("id"))
  {
    return BadRequestError("Id is required");
  }

  String id = server.arg("id");

  JsonArray schedulesJSONArray = schedulesJSONResults.as<JsonArray>();

  bool found = false;

  for (int i = 0; i < (int)schedulesJSONArray.size(); i++)
  {
    JsonDocument scheduleObj = schedulesJSONArray[i];
    String sid = String(scheduleObj["id"] | "");

    if (sid == id)
    {
      schedulesJSONArray.remove(i);
      schedulesFound = schedulesJSONArray.size();
      saveSchedulesToFile();
      found = true;
      break;
    }
  }

  if (found)
  {
    server.send(204, "application/json");
  }
  else
  {
    return BadRequestError("Schedule not found");
  }
}

bool checkForAlarm(byte hora, byte minuto)
{
  String horaStr = String(hora);
  String minutoStr = String(minuto);

  JsonArray schedulesJSONArray = schedulesJSONResults.as<JsonArray>();

  JsonArray devicesJSONArray = devicesJSONResults.as<JsonArray>();

  bool found = false;

  for (int i = 0; i < (int)schedulesJSONArray.size(); i++)
  {
    JsonDocument scheduleObj = schedulesJSONArray[i];
    Schedule schedule = jsonToSchedule(scheduleObj);

    Serial.print("Hora: ");
    Serial.println(schedule.hour);

    Serial.print("Minuto: ");
    Serial.println(schedule.minute);

    if (schedule.hour == horaStr && schedule.minute == minutoStr)
    {
      Serial.println("Alarme encontrado");
      Serial.print("Acao: ");
      Serial.println(schedule.action);
      found = true;

      // Find device referenced by this schedule and apply action
      int idx = findDeviceIndexById(devicesJSONArray, schedule.deviceId);
      if (idx >= 0)
      {
        JsonDocument deviceObj = devicesJSONArray[idx];
        Device device = jsonToDevice(deviceObj);

        String action = schedule.action;

        if (device.main)
        {
          if (action == "0")
          {
            digitalWrite(localPort, LOW); // LED ON
          }
          else if (action == "1")
          {
            digitalWrite(localPort, HIGH); // LED OFF
          }
        }
        else
        {
          // For non-main devices we store the intended status; actual remote command not implemented
        }
      }
      else
      {
        Serial.print("Device not found for id: ");
        Serial.println(schedule.deviceId);
      }
    }
  }

  Serial.println("Alarme checado!");

  return found;
}

void BadRequestError(String message)
{
  JsonDocument doc;

  doc["status"] = "error";
  doc["message"] = message;

  String out;

  serializeJson(doc, out);

  server.send(400, "application/json", out);
}

void handleWebRequests()
{
  if (loadFromSpiffs(server.uri()))
    return;

  String message = "File Not Detected\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for (uint8_t i = 0; i < server.args(); i++)
  {
    message += " NAME:" + server.argName(i) + "\n VALUE:" + server.arg(i) + "\n";
  }

  server.send(404, "text/plain", message);
}
