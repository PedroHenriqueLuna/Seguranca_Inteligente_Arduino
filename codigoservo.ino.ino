#include <WiFi.h>
#include <HTTPClient.h>
#include <UrlEncode.h>
#include <ESP32Servo.h>

const char* ssid = "Pedro"; //wifi
const char* password = "Pedro132"; //senha

String numeroTelefone = "558192483689"; //número do telefone que receberá os alertass
String apikey = "1308594"; 

const int trigPin = 26; 
const int echoPin = 27; 

Servo meuServo; 
int pinServo = 32; 
int angulo = 0;    

unsigned long previousMillis = 0; // Variável para controle do tempo
unsigned long intervalo = 15;      // Intervalo entre os movimentos do servo (ms)
unsigned long intervaloDistancia = 1000; // Intervalo para medir a distância
unsigned long previousMillisDistancia = 0; // Última vez que a distância foi medida

unsigned long previousMillisPausa = 0; // Variável para controle da pausa
bool objetoDetectado = false; // Flag para indicar se o objeto foi detectado

void sendMessage(String message) {
  // Dados a serem enviados com HTTP POST
  String url = "http://api.callmebot.com/whatsapp.php?phone=" + numeroTelefone + "&apikey=" + apikey + "&text=" + urlEncode(message);
  WiFiClient client;
  HTTPClient http;
  http.begin(client, url);


  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  // Enviar requisição HTTP POST
  int httpResponseCode = http.POST(url);

  // Verificar resposta
  if (httpResponseCode == 200) {
    Serial.println("Mensagem enviada com sucesso");
  } else {
    Serial.print("Erro no envio da mensagem. Código de resposta HTTP: ");
    Serial.println(httpResponseCode);
    String response = http.getString();
    Serial.println("Resposta da API: ");
    Serial.println(response);
  }
  http.end();
}

// Função para medir a distância usando o sensor HC-SR04
long measureDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH);
  long distance = (duration * 0.034) / 2; // Cálculo da distância em cm 
  return distance;
}

void setup() {
  Serial.begin(115200);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  meuServo.attach(pinServo); // Configura o pino para o servo

  WiFi.begin(ssid, password);
  Serial.println("Conectando à rede WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Conectado à rede WiFi com endereço IP: ");
  Serial.println(WiFi.localIP());

  // Envia mensagem ao conectar
  sendMessage("Estamos conectados à rede WiFi!");
}

void loop() {
  unsigned long currentMillis = millis();

  // Verificar a distância de forma periódica
  if (currentMillis - previousMillisDistancia >= intervaloDistancia) {
    previousMillisDistancia = currentMillis;
    long distance = measureDistance(); // Medir a distância
    if (distance < 30 && !objetoDetectado) {
      sendMessage("Um objeto foi detectado a " + String(distance) + " cm.");
      objetoDetectado = true; // Definir que o objeto foi detectado
      previousMillisPausa = currentMillis; // Registrar o tempo da pausa
    }
  }

  // Verificar se o objeto foi detectado e aguardar 3 segundos
  if (objetoDetectado) {
    if (currentMillis - previousMillisPausa >= 3000) { // Espera 3 segundos
      objetoDetectado = false; // Permite que o servo volte a girar
    }
  } else {
    // Movimento crescente do servo sem bloquear
    if (currentMillis - previousMillis >= intervalo) {
      previousMillis = currentMillis;

      // Movimento do servo
      if (angulo <= 180) {
        meuServo.write(angulo); // Define o ângulo
        angulo++; // Aumenta o ângulo
      } else {
        angulo = 0; // Se atingir 180°, volta para 0
      }
    }
  }
}