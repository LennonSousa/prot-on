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
#include <LittleFS.h>  //Include File System Headers
#include <SNTPtime.h>
#include <ArduinoJson.h>

// define números de pinos
const int portaLocal = 4;
int dispEncontrados;
int alarmesEncontrados;
const char* imagefile = "/image.png";
const char* htmlfile = "/index.html";
DynamicJsonDocument wifiScanResults(1024);

// Servidor
// Endereço MAC e endereço IP do servidor
//#define SSID "Batista_2-4Ghz"
//#define PASSWORD "celularnove54321"
//#define IP "192.168.0.120"
//#define GATEWAY "192.168.0.1"
//#define SUBNET "255.255.255.0"
#define ArquivoConfigs "/configs.txt"
#define ArquivoDisp "/dispositivos.txt"
#define ArquivoAlarmes "/alarmes.txt"
#define WifiScanResutsFile "/wifi_scan_results.txt"

#define MAX_QTD_DISP 32
#define MAX_QTD_ALARMES 64
#define MAX_SSID_AMOUNT 10

typedef struct {
  String id;
  String nome;
  String ip;
  String fixo;
} Dispositivo;

typedef struct {
  String id;
  String nome;
  String disp;
  String hora;
  String minuto;
  String acao;
  String ativo;
} Alarme;

typedef struct {
  String id;
  String ssid;
  bool secure;
} SSID;

Dispositivo dispositivos[MAX_QTD_DISP];
Alarme alarmes[MAX_QTD_ALARMES];
SSID ssidsFound[MAX_SSID_AMOUNT];

// Variável para informar se o dispositivo já foi configurado
String textoConfig;
String textoDispositivos;
String textoAlarmes;
bool configurado = false;
byte actualMinute = 1;
byte minutoCompara = 0;

ESP8266WebServer server(80);

SNTPtime NTPch("br.pool.ntp.org");
strDateTime dateTime;

void setup() {
  delay(1000);
  Serial.begin(115200);

  //Initialize File System
  if (!LittleFS.begin()) {
    Serial.println("An Error has occurred while mounting LittleFS");
    return;
  }

  textoConfig = carregaArquivo(ArquivoConfigs);
  textoDispositivos = carregaArquivo(ArquivoDisp);
  textoAlarmes = carregaArquivo(ArquivoAlarmes);
  dispEncontrados = qtdDispositivos(textoDispositivos);

  pinMode(portaLocal, OUTPUT);

  //Inicia o ponto de acesso se está configurado
  if (StringContains(textoConfig, "configurado=1")) {
  } else {
    //Muda a configuração para estação e ponto de acesso
    WiFi.mode(WIFI_AP_STA);
    Serial.println("Configurando como soft-AP ... ");
    Serial.println(WiFi.softAP("Prot-On-123456") ? "Ready" : "Failed!");
  }

  //Initialize Webserver
  server.on("/", HTTP_GET, inicio);
  server.on("/status", HTTP_GET, statusLocal);
  // server.on("/estadodispositivos", statusDispositivos);
  server.on("/modifica", HTTP_POST, modificaEstado);
  server.on("/procuraredes", HTTP_GET, procuraRedes);
  server.on("/conectarede", HTTP_POST, conectaRede);
  // server.on("/procuralarmes", procuraAlarmes);
  // server.on("/novoalarme", novoAlarme);
  // server.on("/editaalarme", editarAlarme);
  // server.on("/excluialarme", excluirAlarme);
  // server.onNotFound(handleWebRequests); //Set set
  // server.on("/finalizaconfig", finalizaConfig);
  // server.on("/novodisp", novoDispositivo);
  // server.on("/editadisp", editarDispositivo);
  // server.on("/excluidisp", excluirDispositivo);
  server.onNotFound(handleWebRequests);  //Set setver all paths are not found so we can handle as per URI
  server.begin();
}

void loop() {
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

void inicio() {
  Serial.println("Entrou no inicio");

  if (configurado) {
    Serial.println("Ja configurado");

    server.sendHeader("Location", "/index.html", true);  //Redirect to our html web page
  } else {
    Serial.println("Ainda nao configurado");
    server.sendHeader("Location", "/primeirospassos.html", true);  //Redirect to our html web page
  }

  server.send(302, "text/plane", "");
}

void statusLocal() {
  if (digitalRead(portaLocal) == LOW)
    server.send(200, "application/json", "\{\"status\":0\}");
  else
    server.send(200, "application/json", "\{\"status\":1\}");
}

void modificaEstado() {
  String estadoAtual = "";
  String id = server.arg("id");
  String estadoNovo = server.arg("estado");

  if (estadoNovo == "0") {
    digitalWrite(portaLocal, LOW);  //LED ON
    estadoAtual = "0";              //Feedback parameter
  } else {
    digitalWrite(portaLocal, HIGH);  //LED OFF
    estadoAtual = "1";               //Feedback parameter
  }

  server.send(201, "application/json", "\{\"status\":" + estadoAtual + "\}");  //Send web page
}

void alteraEstado(String id, String estadoNovo) {
  if (id == "0") {
    if (estadoNovo == "0") {
      digitalWrite(portaLocal, LOW);  //LED ON
    } else {
      digitalWrite(portaLocal, HIGH);  //LED OFF
    }
  } else {
  }
}

JsonObject getJSonFromFile(DynamicJsonDocument* doc, String fileName, bool forceCleanONJsonError = true) {
  // open the file for reading:
  String text = carregaArquivo(fileName);

  if (text) {
    DeserializationError error = deserializeJson(*doc, text);
    if (error) {
      // if the file didn't open, print an error:
      Serial.print(F("Error parsing JSON "));
      Serial.println(error.c_str());

      if (forceCleanONJsonError) {
        return doc->to<JsonObject>();
      }
    }

    return doc->as<JsonObject>();
  } else {
    // if the file didn't open, print an error:
    Serial.print(F("Error opening (or file not exists) "));
    Serial.println(fileName);

    Serial.println(F("Empty json created"));
    return doc->to<JsonObject>();
  }
}

bool saveJSonToAFile(DynamicJsonDocument* doc, String fileName) {
  Serial.print(F("Start write..."));

  String newText = "";

  serializeJson(*doc, newText);

  salvaArquivo(newText, fileName, false);

  Serial.print(F("..."));
  // close the file:
  Serial.println(F("done."));

  return true;
}

void procuraRedes() {
  JsonObject obj = getJSonFromFile(&wifiScanResults, WifiScanResutsFile);

  JsonArray results;
  // Check if exist the array
  if (!obj.containsKey(F("results"))) {
    Serial.println(F("Not find results array! Crete one!"));
    
    // results = obj.createNestedArray(F("results"));
    
    WiFi.scanNetworksAsync(printScanResult);

    server.send(204, "application/json");
  } else {
    Serial.println(F("Find results array!"));
    results = obj[F("results")];

    String response = carregaArquivo(WifiScanResutsFile);

    server.send(200, "application/json", response);
  }
}

void printScanResult(int networksFound) {
  Serial.printf("%d network(s) found\n", networksFound);
  for (int i = 0; i < networksFound; i++) {
    Serial.printf("%d: %s, %s\n", i + 1, WiFi.SSID(i).c_str(), WiFi.encryptionType(i) == ENC_TYPE_NONE ? "aberto" : "seguro");
  }

  //Cria um json que inicialmente mostra qual o valor máximo de gpio
  String json = "{\"count\":" + String(networksFound);
  //Lista de pinos
  json += ",\"results\": [";
  for (int i = 0; i < networksFound; i++) {
    //Adiciona no json as informações sobre este pino
    String seguro = WiFi.encryptionType(i) == ENC_TYPE_NONE ? "aberto" : "seguro";
    json += "{";
    json += "\"id\":\"" + String(i + 1) + "\",";
    json += "\"ssid\":\"" + String(WiFi.SSID(i).c_str()) + "\",";
    json += "\"secure\":\"" + String(seguro) + "\"";
    json += "},";
  }
  json += "]}";

  //Remove a última virgula que não é necessário após o último elemento
  json.replace(",]}", "]}");
  //Retorna sucesso e o json
  Serial.printf("Json de redes");
  Serial.println(json);

  // // create an object to add to the array
  // JsonObject objArrayNetworks = results.createNestedObject();

  // for (int i = 0; i < sizeof(objArrayNetworks); i++) {
  //   objArrayNetworks["prevNumOfElem"] = data.size();
  //   objArrayNetworks["newNumOfElem"] = data.size() + 1;
  // }

  salvaArquivo(json, WifiScanResutsFile, false);
}

void conectaRede() {

  String ssidRede = server.arg("ssid");    //Refer  xhttp.open("GET", "setLED?estadoAtual="+led, true);
  String senhaRede = server.arg("senha");  //Refer  xhttp.open("GET", "setLED?estadoAtual="+led, true);

  char ssid[32];
  char senha[64];

  ssidRede.toCharArray(ssid, 32);
  senhaRede.toCharArray(senha, 64);

  //Conecta o ESP ao nosso roteador
  WiFi.begin(ssid, senha);

  Serial.println("");
  Serial.print("Conectando");

  //Espera enquanto não conecta ao roteador
  String retorno = "";

  int y = 0;
  while (WiFi.status() != WL_CONNECTED && y < 15) {
    delay(500);
    Serial.print(".");
    y++;
  }

  if (y < 15) {
    retorno = "http://" + WiFi.localIP().toString();
    Serial.print("Novo IP: ");
    Serial.println(WiFi.localIP());
  }

  if (WiFi.status() == WL_CONNECTED) {
    editaConfiguracao("@ssid", ssidRede, textoConfig, ArquivoConfigs, false);
    editaConfiguracao("@senha", senhaRede, textoConfig, ArquivoConfigs, false);
    editaConfiguracao("@ip0", retorno, textoDispositivos, ArquivoDisp, false);
  }

  WiFi.printDiag(Serial);
  Serial.println(retorno);

  String json = "\{\"baseUrl\":" + retorno + "\}";

  server.send(201, "application/json", json);
}

String carregaArquivo(String caminho) {
  File arquivo;
  String response = "";
  //Se o arquivo existe
  if ((arquivo = LittleFS.open(caminho, "r")) != NULL) {
    Serial.println("");
    Serial.println("Leu o arquivo");
    // read from the file until there's nothing else in it:
    while (arquivo.available()) {
      response += (char)arquivo.read();
    }
    // close the file:
    arquivo.close();
    Serial.println(response);
    Serial.println("");
    return response;
  } else {
    return response;
  }
}

bool salvaArquivo(String textoNovo, String caminho, bool recarrega) {
  File arquivo;
  String configuracoes = "";
  // Se o arquivo existe
  if ((arquivo = LittleFS.open(caminho, "w")) != NULL) {
    // Escreve o novo texto no arquivo
    byte textoBytes[(textoNovo.length() + 1)];

    textoNovo.getBytes(textoBytes, (textoNovo.length() + 1));

    arquivo.write((uint8_t*)textoBytes, sizeof(textoBytes));

    // Fecha o arquivo:
    arquivo.close();

    if (caminho == "/configs.txt")
      textoConfig = carregaArquivo(ArquivoConfigs);
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
  } else {
    Serial.println("Erro ao salvar o arquivo");
    return false;
  }
}

bool StringContains(String texto, String procura) {
  int max = texto.length() - procura.length();
  int tprocura = procura.length();

  for (int i = 0; i <= max; i++) {
    if (texto.substring(i, i + tprocura) == procura) return true;
  }

  return false;
}

int qtdDispositivos(String texto) {
  int qtd = 0;
  int max = texto.length() - 2;

  for (int i = 0; i <= max; i++) {
    if (texto.substring(i, i + 3) == "@id") {
      // Encontrou a configuração solicitada
      qtd += 1;
    }
  }
  return qtd;
}

bool editaConfiguracao(String configuracao, String novo, String texto, String arquivo, bool recarrega) {
  String textoTempInicio;
  String textoTempFinal;
  int max = texto.length() - configuracao.length();
  int tprocura = configuracao.length();
  Serial.println("Editando texto: ");
  Serial.println(texto);

  for (int i = 0; i <= max; i++) {

    if (texto.substring(i, i + tprocura) == configuracao) {
      // Encontrou a configuração solicitada

      int inicioValorConfig = i + (tprocura + 1);

      for (int j = inicioValorConfig; j < texto.length(); j++) {
        if (texto.substring(j, j + 1) == ";") {
          // Armazena o início do texto antes da configuração a ser mudada
          textoTempInicio = texto.substring(0, inicioValorConfig);

          // Armazena o final do texto depois da configuração a ser mudada
          textoTempFinal = texto.substring(j + 1, texto.length());

          textoTempInicio += novo + ";";
          texto = textoTempInicio + textoTempFinal;
          Serial.println("Texto editato: ");
          Serial.println(texto);
          salvaArquivo(texto, arquivo, recarrega);
          return true;
        }
      }
    }
  }

  return false;
}

bool loadFromSpiffs(String path) {
  String dataType = "text/plain";
  if (path.endsWith("/")) path += "index.htm";

  if (path.endsWith(".src")) path = path.substring(0, path.lastIndexOf("."));
  else if (path.endsWith(".html")) dataType = "text/html";
  else if (path.endsWith(".htm")) dataType = "text/html";
  else if (path.endsWith(".css")) dataType = "text/css";
  else if (path.endsWith(".js")) dataType = "application/javascript";
  else if (path.endsWith(".png")) dataType = "image/png";
  else if (path.endsWith(".gif")) dataType = "image/gif";
  else if (path.endsWith(".jpg")) dataType = "image/jpeg";
  else if (path.endsWith(".ico")) dataType = "image/x-icon";
  else if (path.endsWith(".xml")) dataType = "text/xml";
  else if (path.endsWith(".pdf")) dataType = "application/pdf";
  else if (path.endsWith(".zip")) dataType = "application/zip";
  File dataFile = LittleFS.open(path.c_str(), "r");

  if (server.hasArg("download")) dataType = "application/octet-stream";

  if (server.streamFile(dataFile, dataType) != dataFile.size()) {
  }

  dataFile.close();
  return true;
}

void handleWebRequests() {
  if (loadFromSpiffs(server.uri())) return;

  String message = "File Not Detected\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for (uint8_t i = 0; i < server.args(); i++) {
    message += " NAME:" + server.argName(i) + "\n VALUE:" + server.arg(i) + "\n";
  }

  server.send(404, "text/plain", message);
}
