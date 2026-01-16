/*
    This sketch demonstrates how to set up a simple HTTP-like server.
    The server will set a GPIO pin depending on the request
      http://server_ip/gpio/0 will set the GPIO2 low,
      http://server_ip/gpio/1 will set the GPIO2 high
    server_ip is the IP address of the ESP8266 module, will be
    printed to Serial when the module is connected.
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
const int localPort = LED_BUILTIN; // GPIO2
int dispEncontrados;
int alarmesEncontrados;
const char *imagefile = "/image.png";
const char *htmlfile = "/index.html";
JsonDocument wifiScanResults;
JsonDocument settingsJSONResults;
JsonDocument devicesJSONResults;

#define SettingsFile "/settings.json"
#define DevicesFile "/devices.json"
#define ArquivoAlarmes "/schedules.json"
#define WifiScanResutsFile "/wifi_scan_results.json"

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
  UUID id;
  String name;
  UUID deviceId;
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
Device devices[MAX_QTD_DISP];
Schedule schedules[MAX_QTD_ALARMES];
SSID ssidsFound[MAX_SSID_AMOUNT];

// Variável para informar se o dispositivo já foi configurado
String textoConfig;
String textoDispositivos;
String textoAlarmes;
byte actualMinute = 1;
byte minutoCompara = 0;

ESP8266WebServer server(80);

char ntpServer[] = "br.pool.ntp.org";
SNTPtime NTPch(ntpServer);
strDateTime dateTime;

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
      Serial.print(F("Error parsing JSON "));
      Serial.println(error.c_str());

      return doc->to<JsonObject>();
    }

    serializeJson(*doc, Serial);
    Serial.println();

    return *doc;
  }
  else
  {
    Serial.print(F("Error opening (or file not exists) "));
    Serial.println(fileName);

    Serial.println(F("Empty json returned"));

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

void deviceToJSON(const Device &device, JsonDocument &obj)
{
  obj["id"] = device.id;
  obj["name"] = device.name;
  obj["ip"] = device.ip;
  obj["main"] = device.main;
}

void jsonToDevices(JsonArray arr, Device *devices, int maxDevices)
{
  int i = 0;
  for (JsonDocument obj : arr)
  {
    if (i >= maxDevices)
      break;
    devices[i] = jsonToDevice(obj);
    i++;
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

  JsonDocument settingsJSON = getJSONFromFile(&settingsJSONResults, SettingsFile);
  JsonDocument devicesJSON = getJSONFromFile(&devicesJSONResults, DevicesFile);

  settings = jsonToSettings(settingsJSON);
  jsonToDevices(devicesJSON.as<JsonArray>(), devices, MAX_QTD_DISP);

  // textoDispositivos = loadFile(DevicesFile);
  // textoAlarmes = loadFile(ArquivoAlarmes);

  JsonArray arr = devicesJSON.as<JsonArray>();
  dispEncontrados = arr.size();

  pinMode(localPort, OUTPUT);

  listDir("/");

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

  WiFi.onStationModeGotIP([](const WiFiEventStationModeGotIP &evt)
                          {
    Serial.println("Conectado:");
    Serial.println(WiFi.localIP());
    // set NTP, save config, whatever (no blocking here)
    if (NTPch.setSNTPtime()) {
      Serial.println("Hora ajustada");
    } else {
      Serial.println("NTP not set now; will retry later");
      // Option: set another flag to retry in a few seconds
  } });

  // Initialize Webserver
  server.on("/", HTTP_GET, inicio);

  // First Settings
  server.on("/first-setting/device", HTTP_PUT, editDevice);
  server.on("/first-setting/finish", HTTP_POST, finalizaConfig);

  // Wireless
  server.on("/wireless", HTTP_GET, searchWireless);
  server.on("/wireless", HTTP_POST, connectWireless);

  // server.on("/procuralarmes", procuraAlarmes);
  // server.on("/novoalarme", novoAlarme);
  // server.on("/editaalarme", editarAlarme);
  // server.on("/excluialarme", excluirAlarme);

  // Device
  server.on("/device", HTTP_POST, createDevice);
  server.on("/device", HTTP_GET, listDevices);
  server.on("/device", HTTP_PUT, editDevice);
  server.on("/device", HTTP_DELETE, deleteDevice);
  server.on("/device/status", HTTP_PUT, changeDeviceStatus);

  server.onNotFound(handleWebRequests); // Set setver all paths are not found so we can handle as per URI

  server.begin();
}

void loop()
{
  server.handleClient();
  // dateTime = NTPch.getTime(-3, 0); // get time from internal clock
  // actualMinute = dateTime.minute;

  // if (actualMinute != minutoCompara) {

  //   //NTPch.printDateTime(dateTime);

  //   byte actualHour = dateTime.hour;
  //   byte diaDaSemana = dateTime.dayofWeek;

  //   //Serial.print("Minuto atual e minuto compara: ");
  //   //Serial.println(actualMinute);
  //   //Serial.println(minutoCompara);
  //   //Serial.println(diaDaSemana);
  //   checkForAlarm(actualHour, actualMinute);
  //   minutoCompara = actualMinute;
  // }
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
  JsonDocument devicesJSON = getJSONFromFile(&devicesJSONResults, DevicesFile);
  JsonArray devicesJSONArray = devicesJSON.as<JsonArray>();

  for (int i = 0; i < dispEncontrados; i++)
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

  serializeJson(devicesJSONArray, Serial);
  Serial.println();

  const String response = [&]()
  {
    String out;
    serializeJson(devicesJSONArray, out);
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
  JsonDocument devicesJSON = getJSONFromFile(&devicesJSONResults, DevicesFile);
  JsonArray devicesJSONArray = devicesJSON.as<JsonArray>();

  for (int i = 0; i < dispEncontrados; i++)
  {
    JsonDocument deviceObj = devicesJSONArray[i];
    Device device = jsonToDevice(deviceObj);

    if (device.id == id)
    {
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

        break;
      }
      else
      {
        deviceObj["status"] = "error"; // TODO: Default feedback for non-main devices
        jsonResponse = deviceObj;

        break;
      }
    }
  }

  serializeJson(jsonResponse, Serial);
  Serial.println();

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
  Serial.print(F("Start write..."));

  String jsonString = "";

  serializeJson(*doc, jsonString);

  Serial.printf("Json to save");
  Serial.println(jsonString);

  salvaArquivo(jsonString, fileName, false);

  Serial.print(F("..."));
  // close the file:
  Serial.println(F("done."));

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

  int y = 0;
  while (WiFi.status() != WL_CONNECTED && y < 15)
  {
    delay(500);
    Serial.print(".");
    y++;
  }

  if (y < 15)
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

    JsonDocument devicesJSON = getJSONFromFile(&devicesJSONResults, DevicesFile);
    JsonArray devicesJSONArray = devicesJSON.as<JsonArray>();

    for (int i = 0; i < dispEncontrados; i++)
    {
      JsonDocument deviceObj = devicesJSONArray[i];
      Device device = jsonToDevice(deviceObj);

      if (device.main)
      {
        UUID uuid;
        devicesJSONArray[i]["id"] = uuid.toCharArray();
        devicesJSONArray[i]["ip"] = WiFi.localIP().toString();

        saveJsonToAFile(&devicesJSON, DevicesFile);

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

void listDir(const char *dirname)
{
  Serial.printf("Listing directory: %s\n", dirname);

  Dir root = LittleFS.openDir(dirname);

  while (root.next())
  {
    File file = root.openFile("r");
    Serial.print("  FILE: ");
    Serial.print(root.fileName());
    Serial.print("  SIZE: ");
    Serial.print(file.size());
    time_t cr = file.getCreationTime();
    time_t lw = file.getLastWrite();
    file.close();
    struct tm *tmstruct = localtime(&cr);
    Serial.printf("    CREATION: %d-%02d-%02d %02d:%02d:%02d\n", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
    tmstruct = localtime(&lw);
    Serial.printf("  LAST WRITE: %d-%02d-%02d %02d:%02d:%02d\n", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
  }
}

String loadFile(String caminho)
{
  File arquivo = LittleFS.open(caminho, "r");
  String response = "";

  // Se o arquivo existe
  if (arquivo)
  {
    Serial.println("");
    Serial.println("Leu o arquivo");
    // read from the file until there's nothing else in it:
    while (arquivo.available())
    {
      response += (char)arquivo.read();
    }
    // close the file:
    arquivo.close();
    Serial.println(response);
    Serial.println("");
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

    if (caminho == "/configs.txt")
      textoConfig = loadFile(SettingsFile);
    // else if (caminho == "/dispositivos.txt") {
    //   textoDispositivos = loadFile(DevicesFile);
    //   if (recarrega) {
    //     // Atualiza os textos globais
    //     dispEncontrados = qtdDispositivos(textoDispositivos);
    //     pegaDisps(&dispositivos[0]);
    //   }
    // }
    // else if (caminho == "/alarmes.txt") {
    //   textoAlarmes = loadFile(ArquivoAlarmes);
    //   if (recarrega) {
    //     // Atualiza os textos globais
    //     alarmesEncontrados = qtdDispositivos(textoAlarmes);
    //     pegaAlarmes(&alarmes[0]);
    //   }
    // }
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
  if (dispEncontrados >= MAX_QTD_DISP)
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
  JsonDocument devicesJSON = getJSONFromFile(&devicesJSONResults, DevicesFile);
  JsonArray devicesJSONArray = devicesJSON.as<JsonArray>();

  UUID uuid;
  JsonDocument newDeviceObj;

  newDeviceObj["id"] = uuid.toCharArray();
  newDeviceObj["name"] = name;
  newDeviceObj["ip"] = ip;
  newDeviceObj["main"] = false;

  devicesJSONArray.add(newDeviceObj);

  dispEncontrados++;

  saveJsonToAFile(&devicesJSON, DevicesFile);
  deviceToJSON(jsonToDevice(newDeviceObj), jsonResponse);

  serializeJson(jsonResponse, Serial);
  Serial.println();

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
  JsonDocument devicesJSON = getJSONFromFile(&devicesJSONResults, DevicesFile);
  JsonArray devicesJSONArray = devicesJSON.as<JsonArray>();

  for (int i = 0; i < dispEncontrados; i++)
  {
    JsonDocument deviceObj = devicesJSONArray[i];
    Device device = jsonToDevice(deviceObj);

    if (!settings.configured && device.main)
    {
      devicesJSONArray[i]["name"] = name;
      device.name = name;

      saveJsonToAFile(&devicesJSON, DevicesFile);
      deviceToJSON(device, jsonResponse);
      found = true;

      break;
    }
    else if (device.id == id)
    {
      if (nameProvided)
      {
        devicesJSONArray[i]["name"] = name;
        device.name = name;
      }

      if (ipProvided && !device.main)
      {
        devicesJSONArray[i]["ip"] = ip;
        device.ip = ip;
      }

      saveJsonToAFile(&devicesJSON, DevicesFile);
      deviceToJSON(device, jsonResponse);
      found = true;

      break;
    }
  }

  if (!found)
  {
    return BadRequestError("Device not found");
  }

  serializeJson(jsonResponse, Serial);
  Serial.println();

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
  JsonDocument jsonResponse;
  JsonDocument devicesJSON = getJSONFromFile(&devicesJSONResults, DevicesFile);
  JsonArray devicesJSONArray = devicesJSON.as<JsonArray>();

  for (int i = 0; i < dispEncontrados; i++)
  {
    JsonDocument deviceObj = devicesJSONArray[i];
    Device device = jsonToDevice(deviceObj);

    if (device.id == id && !device.main)
    {
      devicesJSONArray.remove(i);
      dispEncontrados--;

      saveJsonToAFile(&devicesJSON, DevicesFile);
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
    return BadRequestError("Device not found or is main device");
  }
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
