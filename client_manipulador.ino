#include <arduino.h>
#include <WiFi.h>

const char* ssid = "VIVOFIBRA-8D50";
const char* password = "184E53AF32";

const char *server_ip = "192.168.15.177";
const uint16_t server_port = 8089;
const uint16_t local_port = 9001; //porta do socket client do manipulador

WiFiClient client;

void setup() {
  Serial.begin(9600);

  // Conectando ao Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Conectando ao Wi-Fi...");
  }
  Serial.println("Conectado ao Wi-Fi!");

  // Tentativa de conectar ao servidor
  if (client.connect(server_ip, server_port)) {
    Serial.println("Conectado ao servidor!");
    client.println("Olá, servidor! Esta é uma mensagem do ESP32.");
  } else {
    Serial.println("Falha ao conectar ao servidor.");
  }
}

void loop() {
  if (client.connected()) {
    // Envia uma mensagem a cada 5 segundos
    client.println("Mensagem do ESP32 ao servidor.");
    delay(5000);
  } else {
    Serial.println("Tentando reconectar...");
    if (client.connect(server_ip, server_port)) {
      Serial.println("Reconectado ao servidor!");
    } else {
      Serial.println("Falha ao reconectar.");
      delay(2000);  // Espera antes de tentar novamente
    }
  }
}
