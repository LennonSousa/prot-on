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
const int portaLocal = 2;
int dispEncontrados;
int alarmesEncontrados;
const char *imagefile = "/image.png";
const char *htmlfile = "/index.html";
JsonDocument wifiScanResults;

#define SettingsFile "/configs.json"
#define ArquivoDisp "/dispositivos.json"
#define ArquivoAlarmes "/alarmes.json"
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
  UUID id;
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
  String text = carregaArquivo(fileName);

  Serial.println(text);

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

    return doc->as<JsonObject>();
  }
  else
  {
    // if the file didn't open, print an error:
    Serial.print(F("Error opening (or file not exists) "));
    Serial.println(fileName);

    Serial.println(F("Empty json created"));
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

  JsonDocument settingsJSON = getJSONFromFile(&wifiScanResults, SettingsFile);

  settings = jsonToSettings(settingsJSON);

  // textoDispositivos = carregaArquivo(ArquivoDisp);
  // textoAlarmes = carregaArquivo(ArquivoAlarmes);
  dispEncontrados = qtdDispositivos(textoDispositivos);

  pinMode(portaLocal, OUTPUT);

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
    WiFi.mode(WIFI_AP_STA);
    Serial.println("Configurando como soft-AP ... ");
    Serial.println(WiFi.softAP("Prot-On-123456") ? "Ready" : "Failed!");
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
  server.on("/status", HTTP_GET, statusLocal);
  server.on("/estadodispositivos", HTTP_GET, statusDispositivos);
  server.on("/modifica", HTTP_POST, modificaEstado);
  server.on("/procuraredes", HTTP_GET, procuraRedes);
  server.on("/conectarede", HTTP_POST, conectaRede);
  // server.on("/procuralarmes", procuraAlarmes);
  // server.on("/novoalarme", novoAlarme);
  // server.on("/editaalarme", editarAlarme);
  // server.on("/excluialarme", excluirAlarme);
  server.onNotFound(handleWebRequests); // Set set
  server.on("/finalizaconfig", finalizaConfig);
  server.on("/novodisp", novoDispositivo);
  server.on("/editadisp", editarDispositivo);
  server.on("/excluidisp", excluirDispositivo);
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

void statusLocal()
{
  if (digitalRead(portaLocal) == LOW)
    server.send(200, "application/json", "{\"status\":0}");
  else
    server.send(200, "application/json", "{\"status\":1}");
}

void statusDispositivos()
{
  // Use ArduinoJson for safe JSON building
  StaticJsonDocument<1024> doc; // Adjust size as needed
  doc["count"] = dispEncontrados;

  JsonArray dispositivosArray = doc.createNestedArray("dispositivos");

  for (int i = 0; i < dispEncontrados; i++)
  {
    JsonObject dispositivo = dispositivosArray.createNestedObject();
    dispositivo["id"] = devices[i].id;
    dispositivo["nome"] = devices[i].name;
    dispositivo["ip"] = devices[i].ip;

    // Envia inicialmente somente o estado do dispositivo local
    String estado;
    if (i == 0)
    {
      // Caso seja o primeiro acesso do cliente, então não tem informação
      if (digitalRead(portaLocal) == 0)
        estado = "0";
      else
        estado = "1";
    }
    // else
    //   estado = verificaOutros(dispositivos[i].ip);

    dispositivo["estado"] = estado;
  }

  String json;
  serializeJson(doc, json);
  server.send(200, "application/json", json);
}

void modificaEstado()
{
  String estadoAtual = "";
  String id = server.arg("id");
  String estadoNovo = server.arg("estado");

  if (estadoNovo == "0")
  {
    digitalWrite(portaLocal, LOW); // LED ON
    estadoAtual = "0";             // Feedback parameter
  }
  else
  {
    digitalWrite(portaLocal, HIGH); // LED OFF
    estadoAtual = "1";              // Feedback parameter
  }

  // Use ArduinoJson for JSON response
  StaticJsonDocument<64> doc;
  doc["status"] = estadoAtual;
  String json;
  serializeJson(doc, json);
  server.send(201, "application/json", json);
}

void alteraEstado(String id, String estadoNovo)
{
  if (id == "0")
  {
    if (estadoNovo == "0")
    {
      digitalWrite(portaLocal, LOW); // LED ON
    }
    else
    {
      digitalWrite(portaLocal, HIGH); // LED OFF
    }
  }
  else
  {
  }
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

void procuraRedes()
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

    String response = carregaArquivo(WifiScanResutsFile);

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

void conectaRede()
{
  String networkSSID = server.arg("ssid");
  String networkPassword = server.arg("password");

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
  String retorno = "";

  int y = 0;
  while (WiFi.status() != WL_CONNECTED && y < 15)
  {
    delay(500);
    Serial.print(".");
    y++;
  }

  if (y < 15)
  {
    retorno = "http://" + WiFi.localIP().toString();
    Serial.print("Novo IP: ");
    Serial.println(WiFi.localIP());
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    JsonDocument settingsJSON = getJSONFromFile(&wifiScanResults, SettingsFile);

    settingsJSON["ssid"] = networkSSID;
    settingsJSON["password"] = networkPassword;

    saveJsonToAFile(&settingsJSON, SettingsFile);

    editaConfiguracao("@ip0", retorno, textoDispositivos, ArquivoDisp, false);
  }

  WiFi.printDiag(Serial);
  Serial.println(retorno);

  // Use ArduinoJson for JSON response
  StaticJsonDocument<128> doc;
  doc["baseUrl"] = retorno;
  String json;
  serializeJson(doc, json);
  server.send(201, "application/json", json);
}

void finalizaConfig()
{
  JsonDocument settingsJSON = getJSONFromFile(&wifiScanResults, SettingsFile);

  settingsJSON["configured"] = true;

  saveJsonToAFile(&settingsJSON, SettingsFile);

  server.send(200, "text/plain", "");
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

String carregaArquivo(String caminho)
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
      textoConfig = carregaArquivo(SettingsFile);
    // else if (caminho == "/dispositivos.txt") {
    //   textoDispositivos = carregaArquivo(ArquivoDisp);
    //   if (recarrega) {
    //     // Atualiza os textos globais
    //     dispEncontrados = qtdDispositivos(textoDispositivos);
    //     pegaDisps(&dispositivos[0]);
    //   }
    // }
    // else if (caminho == "/alarmes.txt") {
    //   textoAlarmes = carregaArquivo(ArquivoAlarmes);
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

bool StringContains(String texto, String procura)
{
  int max = texto.length() - procura.length();
  int tprocura = procura.length();

  for (int i = 0; i <= max; i++)
  {
    if (texto.substring(i, i + tprocura) == procura)
      return true;
  }

  return false;
}

int qtdDispositivos(String texto)
{
  int qtd = 0;
  int max = texto.length() - 2;

  for (int i = 0; i <= max; i++)
  {
    if (texto.substring(i, i + 3) == "@id")
    {
      // Encontrou a configuração solicitada
      qtd += 1;
    }
  }
  return qtd;
}

void idsDispositivos(String texto, String *ids, int qtd)
{
  int max = texto.length() - 5;
  int encontrados = 0;

  for (int i = 0; i <= max; i++)
  {
    if (texto.substring(i, i + 3) == "@id")
    {
      // Encontrou a configuração solicitada
      int inicioValorConfig = i + 3;
      for (int j = inicioValorConfig; j < texto.length(); j++)
      {
        if (texto.substring(j, j + 1) == ";")
        {
          Serial.print("Id encontrada: ");
          Serial.println(texto.substring((i + 3), j));

          ids[encontrados] = texto.substring((i + 3), j);
          encontrados += 1;
          if (encontrados == qtd)
            i += max;

          j += texto.length();
        }
      }
    }
  }
}

String pegaConfiguracao(String configuracao, String texto)
{
  int max = texto.length() - configuracao.length();
  int tprocura = configuracao.length();
  String retorno = "";

  for (int i = 0; i <= max; i++)
  {

    if (texto.substring(i, i + tprocura) == configuracao)
    {

      int inicioValorConfig = i + (tprocura + 1);

      for (int j = inicioValorConfig; j < texto.length(); j++)
      {

        if (texto.substring(j, j + 1) == ";")
        {
          retorno = texto.substring(inicioValorConfig, j);
          return retorno;
        }
      }
    }
  }

  return retorno;
}

bool novaId(int id, String texto, String arquivo, bool recarrega)
{
  String textoTempInicio;
  String textoTempFinal;
  // Armazena o início do texto antes da configuração a ser mudada
  textoTempInicio = texto.substring(0, texto.length());

  // Armazena o final do texto depois da configuração a ser mudada
  textoTempFinal = "@id" + String(id) + ";\n";

  texto = textoTempInicio + textoTempFinal;
  salvaArquivo(texto, arquivo, recarrega);

  return true;
}

bool novaConfiguracao(String configuracao, String novo, String texto, String arquivo, bool recarrega)
{
  String textoTempInicio;
  String textoTempFinal;
  // Armazena o início do texto antes da configuração a ser mudada
  textoTempInicio = texto.substring(0, texto.length());

  // Armazena o final do texto depois da configuração a ser mudada
  textoTempFinal = configuracao + "=" + novo + ";\n";

  texto = textoTempInicio + textoTempFinal;
  salvaArquivo(texto, arquivo, recarrega);

  return true;
}

bool editaConfiguracao(String configuracao, String novo, String texto, String arquivo, bool recarrega)
{
  String textoTempInicio;
  String textoTempFinal;
  int max = texto.length() - configuracao.length();
  int tprocura = configuracao.length();

  for (int i = 0; i <= max; i++)
  {

    if (texto.substring(i, i + tprocura) == configuracao)
    {
      // Encontrou a configuração solicitada

      int inicioValorConfig = i + (tprocura + 1);

      for (int j = inicioValorConfig; j < texto.length(); j++)
      {
        if (texto.substring(j, j + 1) == ";")
        {
          // Armazena o início do texto antes da configuração a ser mudada
          textoTempInicio = texto.substring(0, inicioValorConfig);

          // Armazena o final do texto depois da configuração a ser mudada
          textoTempFinal = texto.substring(j + 1, texto.length());

          textoTempInicio += novo + ";";
          texto = textoTempInicio + textoTempFinal;
          Serial.println("Texto editato: ");
          salvaArquivo(texto, arquivo, recarrega);
          return true;
        }
      }
    }
  }

  return false;
}

bool excluiConfiguracao(String configuracao, String texto, String arquivo, bool recarrega)
{
  String textoTempInicio;
  String textoTempFinal;
  int max = texto.length() - configuracao.length();
  int tprocura = configuracao.length();

  for (int i = 0; i <= max; i++)
  {

    if (texto.substring(i, i + tprocura) == configuracao)
    {
      // Encontrou a configuração solicitada
      for (int j = i; j < texto.length(); j++)
      {
        if (texto.substring(j, j + 1) == ";")
        {
          // Armazena o início do texto antes da configuração a ser mudada
          textoTempInicio = texto.substring(0, i);

          // Armazena o final do texto depois da configuração a ser mudada
          textoTempFinal = texto.substring(j + 1, (texto.length() + 1));
          texto = textoTempInicio + textoTempFinal;
          Serial.println("");
          Serial.print("Texto excluido: ");
          Serial.println(texto);
          salvaArquivo(texto, arquivo, recarrega);
          return true;
        }
      }
    }
  }

  return false;
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

void novoDispositivo()
{
  String nome = server.arg("nome"); // Refer  xhttp.open("GET", "setLED?estadoAtual="+led, true);
  String ip = server.arg("ip");     // Refer  xhttp.open("GET", "setLED?estadoAtual="+led, true);
  String retorno = "0";
  Serial.println("Nome e Ip: ");
  Serial.println(nome);
  Serial.println(ip);

  String idsDips[dispEncontrados];
  idsDispositivos(textoDispositivos, &idsDips[0], dispEncontrados);

  int id = idsDips[dispEncontrados - 1].toInt() + 1;
  novaId(id, textoDispositivos, ArquivoDisp, false);
  novaConfiguracao("@nome" + String(id), nome, textoDispositivos, ArquivoDisp, false);
  novaConfiguracao("@ip" + String(id), ip, textoDispositivos, ArquivoDisp, false);
  novaConfiguracao("@ipfixo" + String(id), "0", textoDispositivos, ArquivoDisp, true);

  server.send(200, "text/plain", retorno); // Send web page
}

void editarDispositivo()
{
  String id = server.arg("id");                       // Refer  xhttp.open("GET", "setLED?estadoAtual="+led, true);
  String novoNome = server.arg("nome");               // Refer  xhttp.open("GET", "setLED?estadoAtual="+led, true);
  String editaCompleto = server.arg("editacompleto"); // Refer  xhttp.open("GET", "setLED?estadoAtual="+led, true);
  String retorno = "0";

  if (editaCompleto == "1")
  {
    String novoIp = server.arg("ip");       // Refer  xhttp.open("GET", "setLED?estadoAtual="+led, true);
    String novoIpFixo = server.arg("fixo"); // Refer  xhttp.open("GET", "setLED?estadoAtual="+led, true);
    editaConfiguracao("@nome" + id, novoNome, textoDispositivos, ArquivoDisp, false);
    editaConfiguracao("@ip" + id, novoIp, textoDispositivos, ArquivoDisp, false);
    editaConfiguracao("@ipfixo" + id, novoIpFixo, textoDispositivos, ArquivoDisp, true);
  }
  else
  {
    editaConfiguracao("@nome" + id, novoNome, textoDispositivos, ArquivoDisp, true);
  }
  server.send(200, "text/plain", retorno); // Send web page
}

void excluirDispositivo()
{
  String id = server.arg("id"); // Refer  xhttp.open("GET", "setLED?estadoAtual="+led, true);
  String retorno = "0";

  excluiConfiguracao("@id" + id, textoDispositivos, ArquivoDisp, false);
  excluiConfiguracao("@nome" + id, textoDispositivos, ArquivoDisp, false);
  excluiConfiguracao("@ip" + id, textoDispositivos, ArquivoDisp, false);
  excluiConfiguracao("@ipfixo" + id, textoDispositivos, ArquivoDisp, true);

  server.send(200, "text/plain", retorno); // Send web page
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
