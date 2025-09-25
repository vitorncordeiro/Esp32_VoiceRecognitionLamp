#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "network";
const char* password = "password";

WebServer server(80);

// LEDs
const int LED0 = 21;   // LED 0 externo
const int LED1 = 22;  // LED 1 externo
const int LED2 = 23;  // LED 2 externo

// Página HTML com card e switches
const char htmlPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="pt-br">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Controle de LEDs</title>
  <style>
    body {
      margin: 0;
      display: flex;
      justify-content: center;
      align-items: center;
      min-height: 100vh;
      background-color: #111827;
      font-family: Arial, sans-serif;
    }
    .card {
      background-color: #1f2937;
      padding: 30px;
      border-radius: 15px;
      width: 300px;
      box-shadow: 0 4px 10px rgba(0,0,0,0.5);
      text-align: center;
      color: white;
    }
    .switch {
      display: flex;
      align-items: center;
      justify-content: space-between;
      margin: 15px 0;
    }
    .switch label {
      cursor: pointer;
    }
    .toggle {
      position: relative;
      width: 50px;
      height: 24px;
    }
    .toggle input {
      opacity: 0;
      width: 0;
      height: 0;
    }
    .slider {
      position: absolute;
      cursor: pointer;
      top: 0;
      left: 0;
      right: 0;
      bottom: 0;
      background-color: #374151;
      transition: 0.3s;
      border-radius: 24px;
    }
    .slider::before {
      position: absolute;
      content: "";
      height: 18px;
      width: 18px;
      left: 3px;
      bottom: 3px;
      background-color: white;
      transition: 0.3s;
      border-radius: 50%;
    }
    .toggle input:checked + .slider {
      background-color: #2596be;
    }
    .toggle input:checked + .slider::before {
      transform: translateX(26px);
    }
  </style>
</head>
<body>
  <div class="card">
    <h2>Controles de LEDs</h2>
    <div class="switch">
      <span>LED 0</span>
      <label class="toggle">
        <input type="checkbox" onchange="toggleLed(0,this.checked)">
        <span class="slider"></span>
      </label>
    </div>
    <div class="switch">
      <span>LED 1</span>
      <label class="toggle">
        <input type="checkbox" onchange="toggleLed(1,this.checked)">
        <span class="slider"></span>
      </label>
    </div>
    <div class="switch">
      <span>LED 2</span>
      <label class="toggle">
        <input type="checkbox" onchange="toggleLed(2,this.checked)">
        <span class="slider"></span>
      </label>
    </div>
  </div>

  <script>
    function toggleLed(led, state) {
      fetch(`/led?num=${led}&state=${state ? 1 : 0}`, { method: 'GET', cache: 'no-cache' });
    }
  </script>
</body>
</html>
)rawliteral";

// Handler da página principal
void handleRoot() {
  server.send(200, "text/html", htmlPage);
}

// Handler dos LEDs
void handleLed() {
  if (server.hasArg("num") && server.hasArg("state")) {
    int led = server.arg("num").toInt();
    int state = server.arg("state").toInt();

    int pin = -1;
    if (led == 0) pin = LED0;
    else if (led == 1) pin = LED1;
    else if (led == 2) pin = LED2;

    if (pin != -1) {
      digitalWrite(pin, state ? HIGH : LOW);
      server.send(200, "text/plain", "OK");
    } else {
      server.send(400, "text/plain", "LED inválido");
    }
  } else {
    server.send(400, "text/plain", "Parâmetros inválidos");
  }
}

void setup() {
  Serial.begin(115200);

  // Configurar pinos como saída
  pinMode(LED0, OUTPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  digitalWrite(LED0, LOW);
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);

  // Conectar Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Conectando ao Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConectado!");
  Serial.print("IP do ESP32: ");
  Serial.println(WiFi.localIP());

  // Configurar rotas do servidor
  server.on("/", handleRoot);
  server.on("/led", handleLed);

  server.begin();
  Serial.println("Servidor iniciado");
}

void loop() {
  server.handleClient();
}
