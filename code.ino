#include <WiFi.h>
#include <HTTPClient.h>
#include <esp_https_server.h>
#include <FirebaseESP32.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// ======================
// CONFIGURAÇÕES DE WIFI
// ======================
const char* ssid = ""; 
const char* password = ""; 

// ======================
// FIREBASE CONFIG
// ======================
#define API_KEY ""
#define DATABASE_URL ""

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// ======================
// RELE CONFIG
// ======================
#define RELE_PIN 4
bool lampadaLigada = false;
bool modoAutomatico = false;
unsigned long tempoLigou = 0;
float potenciaLampada = 7.0; // em Watts 
float precoKwh = 0.70; // valor médio da Copel

// ======================
// NTP CONFIG (hora automática)
// ======================
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", -10800, 60000); // fuso -3h

String getTimestamp() {
  timeClient.update();
  return timeClient.getFormattedTime();
}

// ======================
// CERTIFICADO E CHAVE SSL (os seus)
// ======================
static const char server_cert[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----

-----END CERTIFICATE----- 
)EOF";

static const char server_key[] PROGMEM = R"EOF(
-----BEGIN PRIVATE KEY-----

-----END PRIVATE KEY----- 
)EOF";

// ======================
// HTML DA INTERFACE WEB
// ======================
static const char index_html[] PROGMEM = R"EOF(
<!DOCTYPE html>
<html lang="pt-br">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Controle de Lâmpada com Voz</title>
</head>
<body style="font-family:Arial;text-align:center;background:#f0f0f0;">
  <h1>Controle de Lâmpada com Voz</h1>
  <button onclick="toggleModo()">Alternar Modo (Manual/Automático)</button>
  <p id="modo"></p>
  <button id="micButton">🎤 Falar</button>
  <p id="status">Aguardando...</p>

  <script type="module">
    import { GoogleGenerativeAI } from 'https://esm.run/@google/generative-ai';
    const GEMINI_API_KEY = ""; 
    const genAI = new GoogleGenerativeAI(GEMINI_API_KEY);
    const model = genAI.getGenerativeModel({ model: "gemini-2.5-flash" });
    const systemPrompt = "Eu quero que você identifique essa frase, e se a intenção da frase for de ligar, responda apenas com a palavra 'LIGAR'. Se a intenção for de desligar, responda apenas com a palavra 'DESLIGAR'. Se não for possível identificar claramente, responda apenas com 'NÃO ENTENDI'.";

    const SpeechRecognition = window.SpeechRecognition || window.webkitSpeechRecognition;
    const recognition = new SpeechRecognition();
    recognition.lang = 'pt-BR';
    recognition.continuous = true;
    let isRecording = false;
    let finalTranscript = "";

    document.getElementById("micButton").onclick = () => {
      if (!isRecording) {
        recognition.start();
        isRecording = true;
        document.getElementById("status").innerText = "Ouvindo...";
      } else {
        recognition.stop();
        isRecording = false;
        document.getElementById("status").innerText = "Processando...";
        if (finalTranscript) callGemini(finalTranscript);
      }
    };

    recognition.onresult = (event) => {
      for (let i = event.resultIndex; i < event.results.length; ++i)
        if (event.results[i].isFinal) finalTranscript = event.results[i][0].transcript;
    };

    async function callGemini(texto) {
      const result = await model.generateContent(`${systemPrompt}\nTexto: "${texto}"`);
      const resposta = result.response.text().trim().toUpperCase();
      document.getElementById("status").innerText = "Comando: " + resposta;

      if (resposta === "LIGAR") fetch("/ligar");
      else if (resposta === "DESLIGAR") fetch("/desligar");
    }

    async function toggleModo() {
      await fetch("/toggleModo");
      const r = await fetch("/modo");
      const t = await r.text();
      document.getElementById("modo").innerText = "Modo atual: " + t;
    }

    window.onload = toggleModo;
  </script>
</body>
</html>
)EOF";

// ======================
// HANDLERS HTTP
// ======================
esp_err_t root_get_handler(httpd_req_t *req) {
  httpd_resp_set_type(req, "text/html");
  httpd_resp_send(req, index_html, HTTPD_RESP_USE_STRLEN);
  return ESP_OK;
}

esp_err_t ligar_handler(httpd_req_t *req) {
  digitalWrite(RELE_PIN, HIGH);
  lampadaLigada = true;
  tempoLigou = millis();
  String timestamp = getTimestamp();

  FirebaseJson json;
  json.set("estado", "LIGADA");
  json.set("timestamp", timestamp);
  Firebase.RTDB.pushJSON(&fbdo, "/lampada/historico", json);
  Firebase.RTDB.setString(&fbdo, "/lampada/atual", "LIGADA");

  httpd_resp_sendstr(req, "Lâmpada ligada");
  return ESP_OK;
}

esp_err_t desligar_handler(httpd_req_t *req) {
  digitalWrite(RELE_PIN, LOW);
  lampadaLigada = false;
  unsigned long tempoDesligou = millis();
  float tempoHoras = (tempoDesligou - tempoLigou) / 3600000.0; // ms → h
  float energia = (potenciaLampada * tempoHoras) / 1000.0;
  float custo = energia * precoKwh;

  String timestamp = getTimestamp();

  FirebaseJson json;
  json.set("estado", "DESLIGADA");
  json.set("timestamp", timestamp);
  json.set("tempo_h", tempoHoras);
  json.set("energia_kwh", energia);
  json.set("custo_reais", custo);

  Firebase.RTDB.pushJSON(&fbdo, "/lampada/historico", json);
  Firebase.RTDB.setString(&fbdo, "/lampada/atual", "DESLIGADA");

  httpd_resp_sendstr(req, "Lâmpada desligada");
  return ESP_OK;
}

esp_err_t toggle_modo_handler(httpd_req_t *req) {
  modoAutomatico = !modoAutomatico;
  httpd_resp_sendstr(req, modoAutomatico ? "Modo Automático" : "Modo Manual");
  return ESP_OK;
}

esp_err_t modo_handler(httpd_req_t *req) {
  httpd_resp_sendstr(req, modoAutomatico ? "Automático" : "Manual");
  return ESP_OK;
}

// ======================
// REGISTRO DAS ROTAS
// ======================
httpd_uri_t uri_root = { .uri="/", .method=HTTP_GET, .handler=root_get_handler };
httpd_uri_t uri_ligar = { .uri="/ligar", .method=HTTP_GET, .handler=ligar_handler };
httpd_uri_t uri_desligar = { .uri="/desligar", .method=HTTP_GET, .handler=desligar_handler };
httpd_uri_t uri_toggleModo = { .uri="/toggleModo", .method=HTTP_GET, .handler=toggle_modo_handler };
httpd_uri_t uri_modo = { .uri="/modo", .method=HTTP_GET, .handler=modo_handler };

// ======================
// SERVIDOR HTTPS
// ======================
httpd_handle_t start_https_server(void) {
  httpd_handle_t server = NULL;
  httpd_ssl_config_t conf = HTTPD_SSL_CONFIG_DEFAULT();
  conf.servercert = (const uint8_t*)server_cert;
  conf.servercert_len = strlen(server_cert) + 1;
  conf.prvtkey_pem = (const uint8_t*)server_key;
  conf.prvtkey_len = strlen(server_key) + 1;

  if (httpd_ssl_start(&server, &conf) == ESP_OK) {
    httpd_register_uri_handler(server, &uri_root);
    httpd_register_uri_handler(server, &uri_ligar);
    httpd_register_uri_handler(server, &uri_desligar);
    httpd_register_uri_handler(server, &uri_toggleModo);
    httpd_register_uri_handler(server, &uri_modo);
    Serial.println("Servidor HTTPS iniciado!");
  }
  return server;
}

// ======================
// SETUP E LOOP
// ======================
void setup() {
  Serial.begin(115200);
  pinMode(RELE_PIN, OUTPUT);
  digitalWrite(RELE_PIN, LOW);

  WiFi.begin(ssid, password);
  Serial.print("Conectando");
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\nConectado!");
  Serial.print("Acesse: https://");
  Serial.println(WiFi.localIP());

  timeClient.begin();

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  start_https_server();
}

unsigned long lastAuto = 0;

void loop() {
  if (modoAutomatico && millis() - lastAuto > 15000) {
    lampadaLigada = !lampadaLigada;
    digitalWrite(RELE_PIN, lampadaLigada ? HIGH : LOW);
    String timestamp = getTimestamp();

    FirebaseJson json;
    json.set("estado", lampadaLigada ? "LIGADA (AUTO)" : "DESLIGADA (AUTO)");
    json.set("timestamp", timestamp);
    Firebase.RTDB.pushJSON(&fbdo, "/lampada/historico", json);
    Firebase.RTDB.setString(&fbdo, "/lampada/atual", lampadaLigada ? "LIGADA" : "DESLIGADA");

    lastAuto = millis();
  }

  delay(500);
}
