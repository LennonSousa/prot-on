/*
   ESP8266 SPIFFS HTML Web Page with JPEG, PNG Image

*/

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <FS.h>   //Include File System Headers
#include <SNTPtime.h>

// define números de pinos
const int portaLocal = 2;
int dispEncontrados;
int alarmesEncontrados;
const char* imagefile = "/image.png";
const char* htmlfile = "/index.html";

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
#define MAX_QTD_DISP 32
#define MAX_QTD_ALARMES 64

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

Dispositivo dispositivos[MAX_QTD_DISP];
Alarme alarmes[MAX_QTD_ALARMES];

// Variável para informar se o dispositivo já foi configurado
String textoConfig;
String textoDispositivos;
String textoAlarmes;
bool configurado = false;

ESP8266WebServer server(80);
SNTPtime NTPch("br.pool.ntp.org");
strDateTime dateTime;

void setup() {
  delay(1000);
  Serial.begin(9600);

  //Initialize File System
  SPIFFS.begin();
  textoConfig = carregaArquivo(ArquivoConfigs);
  textoDispositivos = carregaArquivo(ArquivoDisp);
  textoAlarmes = carregaArquivo(ArquivoAlarmes);
  dispEncontrados = qtdDispositivos(textoDispositivos);
  pegaDisps(&dispositivos[0]);

  alarmesEncontrados = qtdDispositivos(textoAlarmes);
  pegaAlarmes(&alarmes[0]);

  pinMode ( portaLocal, OUTPUT );

  //Inicia o ponto de acesso se está configurado
  if (StringContains(textoConfig, "configurado=1")) {
    configurado = true;

    //Muda a configuração para estação
    WiFi.mode(WIFI_STA);
    Serial.println("Configurado como estacao");

    char ssid[32];
    char senha[64];

    pegaConfiguracao("@ssid", textoConfig).toCharArray(ssid, 32);
    pegaConfiguracao("@senha", textoConfig).toCharArray(senha, 64);
    Serial.print("ssid = ");
    Serial.println(ssid);

    Serial.print("senha = ");
    Serial.println(senha);

    //Conecta o ESP ao nosso roteador
    WiFi.begin(ssid, senha);

    Serial.println("");
    Serial.print("Conectando");

    //Espera enquanto não conecta ao roteador
    int y = 0;
    while (WiFi.status() != WL_CONNECTED && y < 30)
    {
      delay(500);
      Serial.print(".");
      y++;
    }

    //Configurações da rede
    //IPAddress ip;
    //IPAddress gateway;
    //IPAddress subnet;
    //ip.fromString(IP);
    //gateway.fromString(GATEWAY);
    //subnet.fromString(SUBNET);

    //Serial.println(ip);

    //Envia para o roteador as configurações que queremos para o ESP
    //WiFi.config(ip, gateway, subnet);
    if (y != 30) {
      Serial.println(WiFi.localIP());

      Serial.println("");
      Serial.println("Conectado");

      while (!NTPch.setSNTPtime()) Serial.print("."); // define o relógio interno
      Serial.println();
      Serial.println("Hora ajustada");
    }
    else {
      Serial.println("Sem conexao");
    }
  }
  else {
    //Muda a configuração para estação e ponto de acesso
    WiFi.mode(WIFI_AP_STA);
    Serial.println("Configurando como soft-AP ... ");
    Serial.println(WiFi.softAP("Prot-On-123456") ? "Ready" : "Failed!");
  }

  //Initialize Webserver
  server.on("/", inicio);
  server.on("/status", statusLocal);
  server.on("/estadodispositivos", statusDispositivos);
  server.on("/modifica", modificaEstado);
  server.on("/procuraredes", procuraRedes);
  server.on("/conectarede", conectaRede);
  server.on("/procuralarmes", procuraAlarmes);
  server.on("/finalizaconfig", finalizaConfig);
  server.on("/novodisp", novoDispositivo);
  server.on("/editadisp", editarDispositivo);
  server.on("/excluidisp", excluirDispositivo);
  server.onNotFound(handleWebRequests); //Set setver all paths are not found so we can handle as per URI
  server.begin();
}

void inicio() {
  Serial.println("Entrou no inicio");

  if (configurado) {
    Serial.println("Ja configurado");
    server.sendHeader("Location", "/index.html", true);  //Redirect to our html web page
  }
  else {
    Serial.println("Ainda nao configurado");
    server.sendHeader("Location", "/primeirospassos.html", true);  //Redirect to our html web page
  }

  server.send(302, "text/plane", "");
}

String carregaArquivo(String caminho) {
  File arquivo;
  String configuracoes = "";
  //Se o arquivo existe
  if ((arquivo = SPIFFS.open(caminho, "r")) != NULL)
  {
    Serial.println("");
    Serial.println("Leu o arquivo");
    // read from the file until there's nothing else in it:
    while (arquivo.available()) {
      configuracoes += (char)arquivo.read();
    }
    // close the file:
    arquivo.close();
    Serial.println(configuracoes);
    Serial.println("");
    return configuracoes;
  }
  else {
    return configuracoes;
  }
}

bool salvaArquivo(String textoNovo, String caminho, bool recarrega) {
  File arquivo;
  String configuracoes = "";
  // Se o arquivo existe
  if ((arquivo = SPIFFS.open(caminho, "w")) != NULL)
  {
    // Escreve o novo texto no arquivo
    byte textoBytes[(textoNovo.length() + 1)];

    textoNovo.getBytes(textoBytes, (textoNovo.length() + 1));

    arquivo.write((uint8_t *) textoBytes, sizeof(textoBytes));

    // Fecha o arquivo:
    arquivo.close();
    textoConfig = carregaArquivo(ArquivoConfigs);
    textoDispositivos = carregaArquivo(ArquivoDisp);
    if (recarrega) {
      // Atualiza os textos globais
      dispEncontrados = qtdDispositivos(textoDispositivos);
      pegaDisps(&dispositivos[0]);
    }

    return true;
  }
  else {
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

String pegaConfiguracao(String configuracao, String texto) {
  int max = texto.length() - configuracao.length();
  int tprocura = configuracao.length();
  String retorno = "";

  for (int i = 0; i <= max; i++) {

    if (texto.substring(i, i + tprocura) == configuracao) {

      int inicioValorConfig = i + (tprocura + 1);

      for (int j = inicioValorConfig; j < texto.length(); j++) {

        if (texto.substring(j, j + 1) == ";") {
          retorno = texto.substring(inicioValorConfig, j);
          return retorno;
        }
      }
    }
  }

  return retorno;
}

bool novaId(int id, String texto, String arquivo, bool recarrega) {
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

bool novaConfiguracao(String configuracao, String novo, String texto, String arquivo, bool recarrega) {
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

bool editaConfiguracao(String configuracao, String novo, String texto, String arquivo, bool recarrega) {
  String textoTempInicio;
  String textoTempFinal;
  int max = texto.length() - configuracao.length();
  int tprocura = configuracao.length();

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
          salvaArquivo(texto, arquivo, recarrega);
          return true;
        }
      }
    }
  }

  return false;
}

bool excluiConfiguracao(String configuracao, String texto, String arquivo, bool recarrega) {
  String textoTempInicio;
  String textoTempFinal;
  int max = texto.length() - configuracao.length();
  int tprocura = configuracao.length();

  for (int i = 0; i <= max; i++) {

    if (texto.substring(i, i + tprocura) == configuracao) {
      // Encontrou a configuração solicitada
      for (int j = i; j < texto.length(); j++) {
        if (texto.substring(j, j + 1) == ";") {
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

void statusLocal() {
  if (digitalRead(portaLocal) == LOW)
    server.send(200, "text/plane", "0");
  else
    server.send(200, "text/plane", "1");
}

void statusDispositivos() {
  // Início
  //Cria um json que inicialmente mostra qual o valor máximo de gpio
  String json = "{\"count\":" + String(dispEncontrados);

  //Lista de dispositivos
  json += ",\"dispositivos\": [";

  for (int i = 0; i < dispEncontrados; i++)
  {
    // Envia inicialmente somente o estado do dispositivo local
    String estado;
    if (i == 0) {
      // Caso seja o primeiro acesso do cliente, então não tem informação
      if (digitalRead(portaLocal) == 0)
        estado = "0";
      else
        estado = "1";
    }
    else
      estado = verificaOutros(dispositivos[i].ip);

    //Adiciona no json as informações sobre este dispositivo
    json += "{";
    json += "\"id\":\"" + dispositivos[i].id + "\",";
    json += "\"nome\":\"" + dispositivos[i].nome + "\",";
    json += "\"ip\":\"" + dispositivos[i].ip + "\",";
    json += "\"estado\":\"" + estado + "\"";
    json += "},";
  }

  json += "]}";
  //Remove a última virgula que não é necessário após o último elemento
  json.replace(",]}", "]}");
  //Retorna sucesso e o json
  server.send(200, "text/json", json);
}

String verificaOutros(String ip) {
  HTTPClient http;  //Declare an object of class HTTPClient
  http.begin("http://" + ip + "/status");  //Specify request destination
  int httpCode = http.GET();
  if (httpCode > 0) { //Check the returning code
    return http.getString();
  }
  http.end();   //Close connection
  return "n/a";
}

void pegaDisps(Dispositivo *disps) {
  // Início
  String idsDips[dispEncontrados];

  idsDispositivos(textoDispositivos, &idsDips[0], dispEncontrados);

  //Lista de dispositivos
  for (int i = 0; i < dispEncontrados; i++)
  {
    //Adiciona no json as informações sobre este dispositivo
    disps[i].id = idsDips[i];
    disps[i].nome = pegaConfiguracao("@nome" + idsDips[i], textoDispositivos);
    disps[i].ip = pegaConfiguracao("@ip" + idsDips[i], textoDispositivos);
    disps[i].fixo = pegaConfiguracao("@ipfixo" + idsDips[i], textoDispositivos);
  }
}

void pegaAlarmes(Alarme *alarms) {
  // Início
  String idsAlarms[alarmesEncontrados];

  idsDispositivos(textoAlarmes, &idsAlarms[0], alarmesEncontrados);

  //Lista de dispositivos
  for (int i = 0; i < alarmesEncontrados; i++)
  {
    //Adiciona no json as informações sobre este dispositivo
    alarms[i].id = idsAlarms[i];
    alarms[i].nome = pegaConfiguracao("@nome" + idsAlarms[i], textoAlarmes);
    alarms[i].disp = pegaConfiguracao("@dispositivo" + idsAlarms[i], textoAlarmes);
    alarms[i].hora = pegaConfiguracao("@hora" + idsAlarms[i], textoAlarmes);
    alarms[i].minuto = pegaConfiguracao("@minuto" + idsAlarms[i], textoAlarmes);
    alarms[i].acao = pegaConfiguracao("@acao" + idsAlarms[i], textoAlarmes);
    alarms[i].ativo = pegaConfiguracao("@ativo" + idsAlarms[i], textoAlarmes);
    Serial.print("ID: ");
    Serial.println(alarms[i].id);
    Serial.print("H: ");
    Serial.println(alarms[i].hora);
    Serial.print("M: ");
    Serial.println(alarms[i].minuto);
  }
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

void idsDispositivos(String texto, String *ids, int qtd) {
  int max = texto.length() - 5;
  int encontrados = 0;

  for (int i = 0; i <= max; i++) {
    if (texto.substring(i, i + 3) == "@id") {
      // Encontrou a configuração solicitada
      int inicioValorConfig = i + 3;
      for (int j = inicioValorConfig; j < texto.length(); j++) {
        if (texto.substring(j, j + 1) == ";") {
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

void modificaEstado() {
  String estadoAtual = "";
  String id = server.arg("id");
  String estadoNovo = server.arg("estado");
  if (estadoNovo == "0")
  {
    digitalWrite(portaLocal, LOW); //LED ON
    estadoAtual = "0"; //Feedback parameter
  }
  else
  {
    digitalWrite(portaLocal, HIGH); //LED OFF
    estadoAtual = "1"; //Feedback parameter
  }

  server.send(200, "text/plane", estadoAtual); //Send web page
}

void alteraEstado(String id, String estadoNovo) {
  if (id == "0") {
    if (estadoNovo == "0") {
      digitalWrite(portaLocal, LOW); //LED ON
    }
    else {
      digitalWrite(portaLocal, HIGH); //LED OFF
    }
  }
  else {

  }
}

void procuraRedes() {
  WiFi.scanNetworksAsync(prinScanResult);
}

void prinScanResult(int networksFound) {
  Serial.printf("%d network(s) found\n", networksFound);
  for (int i = 0; i < networksFound; i++)
  {
    Serial.printf("%d: %s, %s\n", i + 1, WiFi.SSID(i).c_str(), WiFi.encryptionType(i) == ENC_TYPE_NONE ? "aberto" : "seguro");
  }

  //Cria um json que inicialmente mostra qual o valor máximo de gpio
  String json = "{\"count\":" + String(networksFound);
  //Lista de pinos
  json += ",\"redes\": [";
  for (int i = 0; i < networksFound; i++)
  {
    //Adiciona no json as informações sobre este pino
    String seguro = WiFi.encryptionType(i) == ENC_TYPE_NONE ? "aberto" : "seguro";
    json += "{";
    json += "\"numero\":\"" + String(i + 1) + "\",";
    json += "\"ssid\":\"" + String(WiFi.SSID(i).c_str()) + "\",";
    json += "\"seguranca\":\"" + String(seguro) + "\"";
    json += "},";
  }
  json += "]}";
  //Remove a última virgula que não é necessário após o último elemento
  json.replace(",]}", "]}");
  //Retorna sucesso e o json
  server.send(200, "text/json", json);
}

void procuraAlarmes() {
  // Início
  String id = server.arg("idDispAlarme");
  //Cria um json que inicialmente mostra qual o valor máximo de gpio
  String json = "{\"count\":" + String(alarmesEncontrados);

  //Lista de dispositivos
  json += ",\"alarmes\": [";

  for (int i = 0; i < alarmesEncontrados; i++)
  {
    if(alarmes[i].disp == id){
      
    //Adiciona no json as informações sobre este alarme
    json += "{";
    json += "\"id\":\"" + alarmes[i].id + "\",";
    json += "\"nome\":\"" + alarmes[i].nome + "\",";
    json += "\"hora\":\"" + alarmes[i].hora + "\",";
    json += "\"minuto\":\"" + alarmes[i].minuto + "\",";
    json += "\"acao\":\"" + alarmes[i].acao + "\",";
    json += "\"ativo\":\"" + alarmes[i].ativo + "\"";
    json += "},";
  }
  }

  json += "]}";
  //Remove a última virgula que não é necessário após o último elemento
  json.replace(",]}", "]}");
  //Retorna sucesso e o json
  server.send(200, "text/json", json);
}

void conectaRede() {
  String ssidRede = server.arg("ssid"); //Refer  xhttp.open("GET", "setLED?estadoAtual="+led, true);
  String senhaRede = server.arg("senha"); //Refer  xhttp.open("GET", "setLED?estadoAtual="+led, true);

  char ssid[32];
  char senha[64];

  ssidRede.toCharArray(ssid, 32);
  senhaRede.toCharArray(senha, 64);

  //Conecta o ESP ao nosso roteador
  WiFi.begin(ssid, senha);

  Serial.println("");
  Serial.print("Conectando");

  //Espera enquanto não conecta ao roteador
  int y = 0;
  while (WiFi.status() != WL_CONNECTED && y < 15)
  {
    delay(500);
    Serial.print(".");
    y++;
  }

  String retorno = "";
  int httpStatus = 200;
  if (y == 15) {
    retorno = "0";
    httpStatus = 401;
  }
  else {
    retorno = WiFi.localIP().toString();
    Serial.print("Novo IP: ");
    Serial.println(WiFi.localIP());
  }

  if (WiFi.status() == WL_CONNECTED) {
    editaConfiguracao("@ssid", ssidRede, textoConfig, ArquivoConfigs, false);
    editaConfiguracao("@senha", senhaRede, textoConfig, ArquivoConfigs, false);
    editaConfiguracao("@ip0", retorno, textoDispositivos, ArquivoDisp, false);
  }

  // WiFi.localIP()
  server.send(httpStatus, "text/plane", retorno);
}

void finalizaConfig() {
  editaConfiguracao("configurado", "1", textoConfig, ArquivoConfigs, false);

  server.send(200, "text/plane", "");
  delay(1000);

  ESP.restart();
}

void novoDispositivo() {
  String nome = server.arg("nome"); //Refer  xhttp.open("GET", "setLED?estadoAtual="+led, true);
  String ip = server.arg("ip"); //Refer  xhttp.open("GET", "setLED?estadoAtual="+led, true);
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

  server.send(200, "text/plane", retorno); //Send web page
}

void editarDispositivo() {
  String id = server.arg("id"); //Refer  xhttp.open("GET", "setLED?estadoAtual="+led, true);
  String novoNome = server.arg("nome"); //Refer  xhttp.open("GET", "setLED?estadoAtual="+led, true);
  String editaCompleto = server.arg("editacompleto"); //Refer  xhttp.open("GET", "setLED?estadoAtual="+led, true);
  String retorno = "0";

  if (editaCompleto == "1") {
    String novoIp = server.arg("ip"); //Refer  xhttp.open("GET", "setLED?estadoAtual="+led, true);
    String novoIpFixo = server.arg("fixo"); //Refer  xhttp.open("GET", "setLED?estadoAtual="+led, true);
    editaConfiguracao("@nome" + id, novoNome, textoDispositivos, ArquivoDisp, false);
    editaConfiguracao("@ip" + id, novoIp, textoDispositivos, ArquivoDisp, false);
    editaConfiguracao("@ipfixo" + id, novoIpFixo, textoDispositivos, ArquivoDisp, true);
  }
  else {
    editaConfiguracao("@nome" + id, novoNome, textoDispositivos, ArquivoDisp, true);
  }
  server.send(200, "text/plane", retorno); //Send web page
}

void excluirDispositivo() {
  String id = server.arg("id"); //Refer  xhttp.open("GET", "setLED?estadoAtual="+led, true);
  String retorno = "0";

  excluiConfiguracao("@id" + id, textoDispositivos, ArquivoDisp, false);
  excluiConfiguracao("@nome" + id, textoDispositivos, ArquivoDisp, false);
  excluiConfiguracao("@ip" + id, textoDispositivos, ArquivoDisp, false);
  excluiConfiguracao("@ipfixo" + id, textoDispositivos, ArquivoDisp, true);

  server.send(200, "text/plane", retorno); //Send web page
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

byte actualMinute = 1;
byte minutoCompara = 0;

void loop() {
  server.handleClient();
  dateTime = NTPch.getTime(-3, 0); // get time from internal clock
  actualMinute = dateTime.minute;

  if (actualMinute > minutoCompara) {

    //NTPch.printDateTime(dateTime);

    byte actualHour = dateTime.hour;
    byte diaDaSemana = dateTime.dayofWeek;

    Serial.print("Minuto atual e minuto compara: ");
    Serial.println(actualMinute);
    Serial.println(minutoCompara);
    Serial.println(diaDaSemana);
    checkForAlarm(actualHour, actualMinute);
    minutoCompara = actualMinute;
  }
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
  File dataFile = SPIFFS.open(path.c_str(), "r");
  if (server.hasArg("download")) dataType = "application/octet-stream";
  if (server.streamFile(dataFile, dataType) != dataFile.size()) {
  }

  dataFile.close();
  return true;
}

bool checkForAlarm(byte hora, byte minuto)
{
  String horaStr = String(hora);
  String minutoStr = String(minuto);

  for (int x = 0; x < alarmesEncontrados; x++) {
    Serial.print("Hora: ");
    Serial.println(alarmes[x].hora);
    Serial.print("Minuto: ");
    Serial.println(alarmes[x].minuto);
    if (alarmes[x].hora == horaStr && alarmes[x].minuto == minutoStr) {
      Serial.println("Alarme encontrado");
      alteraEstado(alarmes[x].disp, alarmes[x].acao);
    }
  }

  Serial.println("Alarme checado!");
}
