#include <WiFi.h>
#include <esp_https_server.h>
#include "esp_log.h" 

// ======================
// PINO DO RELÉ
// ======================
// Define o pino onde o relé está conectado
#define RELAY_PIN 4

// Define os estados do relé. 
// A maioria dos módulos de relé é "Active-LOW", 
// o que significa que LOW liga o relé e HIGH desliga.
// Se o seu relé for o oposto, inverta os valores.
#define RELAY_ON  HIGH
#define RELAY_OFF LOW

// ======================
// PINOS DOS LEDS (NOVO)
// ======================
#define LED_VERDE_PIN 12    // <-- NOVO: LED de status do WiFi (GPIO 5)
#define LED_AMARELO_PIN 13 // <-- NOVO: LED de status do Relé (GPIO 18)

// --- Estados dos LEDs ---
// (LEDs padrão: HIGH = LIGADO)
#define LED_ON  HIGH       // <-- NOVO
#define LED_OFF LOW        // <-- NOVO


// ======================
// Credenciais de WiFi
// ======================
const char* ssid = "iPhone de Eduardo";      // (Seu SSID)
const char* password = "80230070"; // (Sua Senha)

// ======================
// Certificado e chave
// ======================
static const char server_cert[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIEKzCCAxOgAwIBAgIULVrUaZxMGXLraorVV/VUalOjlbUwDQYJKoZIhvcNAQEL
BQAwgaQxCzAJBgNVBAYTAkJSMQ8wDQYDVQQIDAZQYXJhbmExETAPBgNVBAcMCEN1
cml0aWJhMRkwFwYDVQQKDBBNZXVzZXJ2aWRvckVTUDMyMRQwEgYDVQQLDAtFU1Az
Mi5sb2NhbDEVMBMGA1UEAwwMRVNQMzIuc2VydmVyMSkwJwYJKoZIhvcNAQkBFhpl
ZHVhcmRvLmJsYXNjemFrQGdtYWlsLmNvbTAeFw0yNTExMDUxMzA1MzJaFw0yNjEx
MDUxMzA1MzJaMIGkMQswCQYDVQQGEwJCUjEPMA0GA1UECAwGUGFyYW5hMREwDwYD
VQQHDAhDdXJpdGliYTEZMBcGA1UECgwQTWV1c2Vydmlkb3JFU1AzMjEUMBIGA1UE
CwwLRVNQMzIubG9jYWwxFTATBgNVBAMMDEVTUDMyLnNlcnZlcjEpMCcGCSqGSIb3
DQEJARYaZWR1YXJkby5ibGFzY3pha0BnbWFpbC5jb20wggEiMA0GCSqGSIb3DQEB
AQUAA4IBDwAwggEKAoIBAQCbmgppLDigo4yZqCS5gcQSlPr+K4MFSzL7Lraqs3bV
uGgwi4/+uJb6vCyUx7nW1TcjPY/NtLVCG+skZYrIqyj3UJ7Fs1BTxZu2nA7fl0Ol
R9Q2TycRd2majWR4ASSNk8LUIdfuAWAHVAEDpeRzyKBUZcEsX9yUnmr9VoYuc6zv
w8vtX0QEAA2tRGS/14ogRg5+SuGBi9LbyOV/1SNCE6SfDbQKKAT4kJYGP8A8Lfl9
qF/awqPppcvFK37vAlJRoqPOmHK4XTBvX9+R2Zvc3MVIE7xFYzGGuhI+31mRgUsH
G8X0oLXGf01HqE9wSpkem8fKXfKauJxhoLqxRBL0suudAgMBAAGjUzBRMB0GA1Ud
DgQWBBQV58BvejzIfzDhrkV4PHuvSV/xPDAfBgNVHSMEGDAWgBQV58BvejzIfzDh
rkV4PHuvSV/xPDAPBgNVHRMBAf8EBTADAQH/MA0GCSqGSIb3DQEBCwUAA4IBAQAA
yo4SytA15HrFLaWgPWW/Z+kjCSMC3fU98Rc3YjqvhxCfrNgRTK/mx/jvc9YFF8ku
pmF+c3GJCBEgu7Xe4XO+kFljilOua19ubExfyvDZj+0YQyztqzr+DQC9zMYgc1op
RVTbyEJs4QtNG9jAFhYHFNa6o9mjJAgfx2WScXg0KPxqGhQcLRqGllKTnvtdXLbj
jxouwZzRyeRs6oRT3JfPQ14kKzOcK+SjgB30um1SD84M2unnHynSZ3k373cTxOjo
jHDQ5FpFEHA7PhkEB+Wg5boyPwXIwfAldd/YM8qI3mpgiIhUgCdND3Kf6oHNRmo4
8wI+FWEN9MMSGtgdvZ94
-----END CERTIFICATE-----
)EOF";

static const char server_key[] PROGMEM = R"EOF(
-----BEGIN PRIVATE KEY-----
MIIEvgIBADANBgkqhkiG9w0BAQEFAASCBKgwggSkAgEAAoIBAQCbmgppLDigo4yZ
qCS5gcQSlPr+K4MFSzL7Lraqs3bVuGgwi4/+uJb6vCyUx7nW1TcjPY/NtLVCG+sk
ZYrIqyj3UJ7Fs1BTxZu2nA7fl0OlR9Q2TycRd2majWR4ASSNk8LUIdfuAWAHVAED
peRzyKBUZcEsX9yUnmr9VoYuc6zvw8vtX0QEAA2tRGS/14ogRg5+SuGBi9LbyOV/
1SNCE6SfDbQKKAT4kJYGP8A8Lfl9qF/awqPppcvFK37vAlJRoqPOmHK4XTBvX9+R
2Zvc3MVIE7xFYzGGuhI+31mRgUsHG8X0oLXGf01HqE9wSpkem8fKXfKauJxhoLqx
RBL0suudAgMBAAECggEAB6SKTF4qreE6n7V9kEsKaWs95MwYVGM8LtA9kqbb+GVl
YLUKLpW8fLZPbkNdIms4mReF/7GXkZazdwcxyOd3TKZ+FfHXyTWUj2Sc6miQnwan
Kr+6iIYX/sphG+/absXKNQcwhvU0VUfQAPDeBr3c+ihBU7+lovshAC6GyrNN1Dzz
JQiBnVhOpIZm0y76YaCa+Ct0+yCnRchCzl2uhcn9N3od8dIWzzlNflpBsY6Fs8Sd
zSx9k/QaUGgveknW12mEtyyYNV0oUG6YKTNSbAKcOqzbEBxFFEw3annyF+PGpMIw
F16RUTbI/VHdM1ghOljkqhZRjNLN5eYD6JktG9U1gQKBgQDHnAoSuvKyZcyqEWEr
XPU3Rg04Dr2V9jK2xWlZ+HYG6z+AmUhqtyEjGSUU9tmrawNRSHz8IsgJzqIA28s9
Iza6IEwGimjTwBgeJqEj0c/nWe4PBM8v1yyJlDGW3aD6y1f39kVzLyPYh2qYybWc
+u2y4j+YHQEQjbKd/ICPgVpCPQKBgQDHj1ANHK6PdncQrx1BDQoDdH0xUKNC06aE
ujkizGXyNA6qGfJmiaM+dxrTDLA38OZN0Aq+uZxI9fDO6soy55z4qBIZJ+GefKDb
6vu9XXBB9Brhvkj/LN17aPXEdyCb5sNW5Wlw5M8nH5tGvQ/jr7gFjzU08VJ9Dy9r
eCgjssjE4QKBgA656VhWaH9W2VGWtKeBVSn9xg3jsIL2xekMCWi01uhNxMKVDG2r
EwvG2CCC0a4+1+DQS0BCxKVMlFEh5g2donT62wEEhxVQL7dRvEOrP/5eeRysiyiJ
ZGTGBR6PPUgjZlsJ1TfZO3jHDMs23mo1bQHshSqQwTpaadoT/dwYe/M1AoGBAKc9
gH3smpIaWsA9sUuR7OFmX4DTrm7Tx0qWckmqeIlZfSyq4A10rb0sves/R/MpzShL
AKZEESjIDA6sj9XHazKsT7aLSLa0hW6oaF6Tjv+G3nxvJufLJrZFCSqkF0zwQKZz
TkFRUBXelBfdXBKZcaQfFx0OXO33qurq4ODB9SWhAoGBAMWXbxw2d9i52k/p/fEv
Zo70bAOPUj8Txa0TUpWQM9XnT8fziqSYuAPMeTwS5v5g6vckUXb57aPRuMvz+Dmf
sJJb6tuStI9JU/jwss6BY8uAfvDdarfsxQMpCj2ZU+mFQyV0afXPY0k9Mf/Kjmqb
Vyl8fe4x1acDnQWqOt0iLCZA
-----END PRIVATE KEY-----
)EOF";

// =============================================
// A Página HTML com JavaScript (Sem alterações)
// =============================================
static const char index_html[] PROGMEM = R"EOF(
<!DOCTYPE html>
<html lang="pt-br">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 Controle de Voz (Gemini)</title>
    <style>
        /* [O CSS permanece o mesmo] */
        body { font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif; text-align: center; background: #f0f2f5; margin-top: 50px; color: #333; }
        h1 { color: #1e1e1e; }
        #micButton { font-size: 1.5em; padding: 20px 30px; cursor: pointer; border: none; border-radius: 50px; background-color: #007aff; color: white; box-shadow: 0 4px 12px rgba(0,0,0,0.1); transition: background-color 0.2s, transform 0.2s; }
        #micButton.recording { background-color: #ff3b30; animation: pulse 1.5s infinite; }
        @keyframes pulse { 0% { box-shadow: 0 0 0 0 rgba(255, 59, 48, 0.7); } 70% { box-shadow: 0 0 0 20px rgba(255, 59, 48, 0); } 100% { box-shadow: 0 0 0 0 rgba(255, 59, 48, 0); } }
        #transcript { font-size: 1.2em; margin-top: 30px; min-height: 100px; max-width: 600px; margin-left: auto; margin-right: auto; border: 1px solid #ddd; background: #ffffff; padding: 20px; border-radius: 8px; text-align: left; }
        #transcript i { color: #999; }
        #status { font-size: 1.1em; color: #555; margin-top: 20px; font-style: italic; }
    </style>
</head>
<body>
    <h1>Controle de Voz ESP32 (Gemini)</h1>
    <p>Clique no botão e comece a falar. Clique em parar para processar.</p>
    <button id="micButton">▶ Falar</button>
    <div id="transcript">...</div>
    <div id="status">Aguardando comando.</div>

    <script type="module">
        // ======================================
        // IMPORTA A BIBLIOTECA DO GEMINI
        // ======================================
        import { GoogleGenerativeAI } from 'https://esm.run/@google/generative-ai';

        // ======================================
        // CONFIGURAÇÃO DA API DA LLM
        // ======================================
        // ❗ (Sua Chave de API)
        const GEMINI_API_KEY = "AIzaSyByq5zji1RG240zSODkYS8JTbJmP8xhLlM"; 
        
        // ==========================================================
        // PROMPT DO SISTEMA
        // ==========================================================
        const systemPrompt = "Eu quero que você identifique essa frase, e se a intenção da frase for de ligar, responda apenas com a palavra 'LIGAR'. Se a intenção for de desligar, responda apenas com a palavra 'DESLIGAR'. Se não for possível identificar claramente, responda apenas com 'NÃO ENTENDI'.";

        const genAI = new GoogleGenerativeAI(GEMINI_API_KEY);
        const model = genAI.getGenerativeModel({ 
            model: "gemini-2.5-flash"
        });

        // ======================================
        // ELEMENTOS DO HTML
        // ======================================
        const btn = document.getElementById('micButton');
        const output = document.getElementById('transcript');
        const statusEl = document.getElementById('status');
        let isRecording = false;

        // ======================================
        // CONFIGURAÇÃO DO SPEECH RECOGNITION
        // ======================================
        const SpeechRecognition = window.SpeechRecognition || window.webkitSpeechRecognition;
        let recognition;

        if (!SpeechRecognition) {
            output.innerHTML = "Seu navegador não suporta a API de Reconhecimento de Fala. Tente o Chrome ou Edge.";
            btn.disabled = true;
        } else {
            recognition = new SpeechRecognition();
            recognition.continuous = true;
            recognition.interimResults = true;
            recognition.lang = 'pt-BR';

            let finalTranscript = '';

            recognition.onresult = (event) => {
                let interimTranscript = '';
                finalTranscript = ''; 

                for (let i = event.resultIndex; i < event.results.length; ++i) {
                    const transcriptPart = event.results[i][0].transcript;
                    if (event.results[i].isFinal) {
                        finalTranscript += transcriptPart;
                    } else {
                        interimTranscript += transcriptPart;
                    }
                }
                output.innerHTML = finalTranscript + '<i style="color:gray;">' + interimTranscript + '</i>';
            };

            recognition.onend = () => {
                isRecording = false;
                btn.innerHTML = '▶ Falar';
                btn.classList.remove('recording');
            };
            
            recognition.onerror = (event) => {
                console.error("Erro no SpeechRecognition: ", event.error);
                if (event.error === 'not-allowed') {
                    statusEl.innerText = "Permissão ao microfone negada.";
                } else if (event.error === 'no-speech') {
                    statusEl.innerText = "Nenhuma fala detectada.";
                }
            };

            // ======================================
            // LÓGICA PRINCIPAL (Botão e API)
            // ======================================
            btn.onclick = () => {
                if (isRecording) {
                    // --- PARAR DE GRAVAR ---
                    recognition.stop();
                    statusEl.innerText = "Processando...";
                    
                    if (finalTranscript) {
                        callGemini(finalTranscript);
                    } else {
                        statusEl.innerText = "Nenhum texto capturado. Tente novamente.";
                    }

                } else {
                    // --- COMEÇAR A GRAVAR ---
                    finalTranscript = ''; 
                    output.innerHTML = 'Ouvindo...';
                    statusEl.innerText = "Fale agora.";
                    try {
                        recognition.start();
                        isRecording = true;
                        btn.innerHTML = '⏹ Parar';
                        btn.classList.add('recording');
                    } catch (err) {
                        console.error("Erro ao tentar iniciar: ", err);
                        statusEl.innerText = "Erro ao iniciar. Tente novamente.";
                    }
                }
            };
        }

        // ======================================
        // FUNÇÃO DA LLM (GEMINI)
        // ======================================
        async function callGemini(userText) {
            
            try {
                const fullPrompt = `${systemPrompt}\n\nTexto do usuário: "${userText}"`;
                
                const result = await model.generateContent(fullPrompt);
                const response = result.response;
                
                let command = response.text().trim().toUpperCase();
                console.log('Resposta bruta da LLM:', response.text());
                
                statusEl.innerText = `Comando recebido: ${command}`;
                handleCommand(command);

            } catch (error) {
                console.error('Erro ao chamar Gemini:', error);
                statusEl.innerText = 'Erro ao conectar com a IA.';
                if (error.message.includes('API key not valid')) {
                    statusEl.innerText = 'Erro: Chave de API do Gemini inválida!';
                } else if (error.message.includes('400')) {
                     statusEl.innerText = 'Erro 400: Requisição mal formatada. Verifique sua Chave de API.';
                }
            }
        }

        // ==========================================================
        // FUNÇÃO FETCH PARA CONTROLAR O RELÉ
        // ==========================================================
        async function sendRelayCommand(endpoint) {
            try {
                // Faz a requisição para o ESP32 (ex: /ligar ou /desligar)
                const response = await fetch(endpoint); 
                
                if (!response.ok) {
                    throw new Error(`Erro na requisição: ${response.statusText}`);
                }
                
                // Pega a resposta (ex: "Relay LIGADO") e mostra no status
                const resultText = await response.text();
                console.log(`Resposta do ESP32: ${resultText}`);
                statusEl.innerText = resultText; // Atualiza o status com a resposta do ESP

            } catch (error) {
                console.error('Erro ao enviar comando para o ESP32:', error);
                statusEl.innerText = 'Erro ao controlar o relé.';
            }
        }


        // ==========================================================
        // O BLOCO IF/ELSE
        // ==========================================================
        function handleCommand(command) {
            console.log('Comando processado pela LLM:', command);
            
            if (command === 'LIGAR') {
                console.log('AÇÃO: LIGAR');
                statusEl.innerText = 'Enviando comando LIGAR...';
                sendRelayCommand('/ligar'); 
                
            } else if (command === 'DESLIGAR') {
                console.log('AÇÃO: DESLIGAR');
                statusEl.innerText = 'Enviando comando DESLIGAR...';
                sendRelayCommand('/desligar'); 
                
            } else { // 'NÃO ENTENDI'
                console.log('AÇÃO: NÃO ENTENDI');
                statusEl.innerText = 'Comando não reconhecido. Tente novamente.'; 
            }
        }
    </script>
</body>
</html>
)EOF";


// ======================
// Handler da página ROOT
// ======================
esp_err_t root_get_handler(httpd_req_t *req)
{
    // Define o tipo de conteúdo como HTML
    httpd_resp_set_type(req, "text/html");
    
    // Envia a página HTML completa
    httpd_resp_send(req, index_html, HTTPD_RESP_USE_STRLEN);
    
    return ESP_OK;
}

httpd_uri_t uri_root = {
    .uri      = "/",
    .method   = HTTP_GET,
    .handler  = root_get_handler,
    .user_ctx = NULL
};


// =======================================
// Handlers do RELÉ (MODIFICADOS COM LEDs)
// =======================================

// Handler para /ligar
esp_err_t ligar_get_handler(httpd_req_t *req)
{
    Serial.println("Recebido comando HTTP /ligar");
    digitalWrite(RELAY_PIN, RELAY_ON);     // <-- CORRIGIDO (Usa RELAY_ON para ligar)
    digitalWrite(LED_AMARELO_PIN, LED_OFF); // <-- NOVO: Acende LED amarelo
    
    // Responde ao navegador (JavaScript) que o comando foi recebido
    httpd_resp_set_type(req, "text/plain");
    httpd_resp_send(req, "Relé LIGADO", HTTPD_RESP_USE_STRLEN);
    
    return ESP_OK;
}

// Handler para /desligar
esp_err_t desligar_get_handler(httpd_req_t *req)
{
    Serial.println("Recebido comando HTTP /desligar");
    digitalWrite(RELAY_PIN, RELAY_OFF);     // <-- CORRIGIDO (Usa RELAY_OFF para desligar)
    digitalWrite(LED_AMARELO_PIN, LED_ON); // <-- NOVO: Apaga LED amarelo
    
    // Responde ao navegador (JavaScript)
    httpd_resp_set_type(req, "text/plain");
    httpd_resp_send(req, "Relé DESLIGADO", HTTPD_RESP_USE_STRLEN);
    
    return ESP_OK;
}

// =======================================
// Definições das URIs do Relé
// =======================================

httpd_uri_t uri_ligar = {
    .uri      = "/ligar",
    .method   = HTTP_GET,
    .handler  = ligar_get_handler, // Chama a função ligar
    .user_ctx = NULL
};

httpd_uri_t uri_desligar = {
    .uri      = "/desligar",
    .method   = HTTP_GET,
    .handler  = desligar_get_handler, // Chama a função desligar
    .user_ctx = NULL
};


// ===================================
// Inicializa o servidor HTTPS
// ===================================
httpd_handle_t start_https_server(void)
{
    httpd_handle_t server = NULL;
    httpd_ssl_config_t conf = HTTPD_SSL_CONFIG_DEFAULT();

    // Converte strings para ponteiros de bytes
    conf.servercert = (const uint8_t *)server_cert;
    conf.servercert_len = strlen(server_cert) + 1; 
    conf.prvtkey_pem = (const uint8_t *)server_key;
    conf.prvtkey_len = strlen(server_key) + 1;

    if (httpd_ssl_start(&server, &conf) == ESP_OK) {
        // Registra o handler principal (página)
        httpd_register_uri_handler(server, &uri_root);
        
        // Registra os novos handlers do relé
        httpd_register_uri_handler(server, &uri_ligar);
        httpd_register_uri_handler(server, &uri_desligar);
        
        Serial.println("Servidor HTTPS iniciado com sucesso.");
    } else {
        Serial.println("Falha ao iniciar servidor HTTPS!");
    }
    return server;
}

// ======================
// Setup e Loop (MODIFICADO COM LEDs)
// ======================
void setup(void)
{
    Serial.begin(115200);
    Serial.println();

    // Configura o pino do relé
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, RELAY_OFF); // Garante que o relé começa desligado
    
    // Configura os pinos dos LEDs (NOVO)
    pinMode(LED_VERDE_PIN, OUTPUT);     // <-- NOVO
    pinMode(LED_AMARELO_PIN, OUTPUT);   // <-- NOVO
    
    // Garante que ambos os LEDs comecem apagados (NOVO)
    digitalWrite(LED_VERDE_PIN, LED_OFF);   // <-- NOVO
    digitalWrite(LED_AMARELO_PIN, LED_OFF); // <-- NOVO

    Serial.println("Pinos de I/O configurados.");


    Serial.println("Conectando ao WiFi...");
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        // (O LED verde permanece desligado aqui)
    }

    Serial.println("\nWiFi conectado!");
    Serial.print("IP: https://"); 
    Serial.println(WiFi.localIP());

    // Acende o LED verde para indicar conexão (NOVO)
    digitalWrite(LED_VERDE_PIN, LED_ON); // <-- NOVO

    start_https_server();
}

void loop(void) 
{
    // O servidor roda em segundo plano, loop pode ficar vazio.
    delay(1000);
}
