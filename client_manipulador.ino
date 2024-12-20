#include <WiFi.h>
#include <lwip/sockets.h>
#include <Arduino_JSON.h>
#include <Servo.h>
#include "HX711.h"

/*------------------------------------------------------------------------------ 
                            SEÇÃO DO WIFI E SOCKETS
------------------------------------------------------------------------------*/

// Variáveis para conexão com WiFi
const char* ssid = "VIVOFIBRA-8D50"; // Mudar de acordo com o WiFi que estiver utilizando
const char* password = "184E53AF32"; // Mudar para senha do WiFi correspondente

// Variáveis para conexão com servidor
const char* server_ip = "192.168.15.177";
const uint16_t server_port = 8089;
const uint16_t local_port = 9091; // Porta local desejada

int sock; // Socket do cliente

// Conexão com WiFi
void wifiConnection(){
  // Conectando ao Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Conectando ao Wi-Fi...");
  }
  Serial.println("Conectado ao Wi-Fi!");
}

// Conexão com Socket Server Storage.py
void socketConnection(){
  // Criando um socket
  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    Serial.println("Falha ao criar o socket.");
    while (true);
  }

  // Ativando reutilização da porta
  int opt = 1;
  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
    Serial.println("Falha ao configurar SO_REUSEADDR.");
    close(sock);
    while (true);
  }

  // Associando o socket à porta local específica
  struct sockaddr_in localAddr;
  memset(&localAddr, 0, sizeof(localAddr)); // Certifique-se de zerar a estrutura
  localAddr.sin_family = AF_INET;
  localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  localAddr.sin_port = htons(local_port);

  if (bind(sock, (struct sockaddr*)&localAddr, sizeof(localAddr)) < 0) {
    Serial.println("Falha ao associar a porta local. Certifique-se de que a porta não está em uso.");
    close(sock);
    while (true);
  }
  Serial.print("Socket associado à porta local: ");
  Serial.println(local_port);

  // Configurando o endereço do servidor
  struct sockaddr_in serverAddr;
  memset(&serverAddr, 0, sizeof(serverAddr)); // Certifique-se de zerar a estrutura
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(server_port);
  inet_pton(AF_INET, server_ip, &serverAddr.sin_addr.s_addr);

  // Conectando ao servidor
  if (connect(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
    Serial.println("Falha ao conectar ao servidor.");
    close(sock);
    while (true);
  }
  Serial.println("Conectado ao servidor!");

  // Enviando uma mensagem inicial
  const char* msg = "Olá, servidor! Esta é uma mensagem do ESP32 via TCP.";
  send(sock, msg, strlen(msg), 0);
}

/*------------------------------------------------------------------------------ 
                        SEÇÃO DO TRATAMENTO DOS DADOS
------------------------------------------------------------------------------*/
int receiveAndProcessData(int sock){
  const size_t jsonBufferSize = 1024;
  char jsonBuffer[jsonBufferSize];

  memset(jsonBuffer, 0, jsonBufferSize);

  int bytesReceived = recv(sock, jsonBuffer, jsonBufferSize - 1, 0);
  if(bytesReceived < 0){
    Serial.println("Erros ao receber dados do servidor");
    return;
  } else if (bytesReceived == 0){
    Serial.println("Erros ao receber dados do servidor");
    return; 
  }

  // Deserializando o JSON
  JSONVar jsonObject = JSON.parse(jsonBuffer);

  // Verifica se houve erro na Deserialização
  if(JSON.typeof(jsonObject) == "undefined"){
    Serial.println("Erro ao analisar JSON recebido");
    return;
  }

  // Exemplo: Acessando um campo específico
  if (jsonObject.hasOwnProperty("product_num")) {
    int product = (int)jsonObject["product_num"];
    return product;
  } else{
    Serial.println("Propriedade 'product_num' não encontrada no JSON");
  }
}

void createAndSendData(int sock, int qty, int product_num, int operation){
  /*-------------------------------------------------------------------------------- 
    Lógica para criar objeto json e enviar pelo socket conforme alteração na balança
  ---------------------------------------------------------------------------------*/
  DynamicJsonDocument doc(512);
  doc['operation'] = operation;
  doc['product_num'] = product_num
  doc['qty'] = qty

  String jsonString;
  serializeJson(doc, jsonString)
  Serial.println(jsonString)
  
  send(sock, jsonString, strlen(jsonString), 0);
}


/*------------------------------------------------------------------------------ 
                          SEÇÃO DE CONTROLE DO ROBÔ
------------------------------------------------------------------------------*/

// Define os servos
Servo servo1;
Servo servo2;
Servo servo3;
Servo microServo1;
Servo microServo2;
Servo microServo3;

// Posição neutras para os servos
int posicao_neutra[6] = {90, 90, 90, 90, 90, 80};

// Função para mover até coordenadas definidas
void moverParaCoordenadas(int coordenadas[6]) {
  moverServoSuavemente(servo1, servo1.read(), coordenadas[0]);
  delay(2000);  // Aguarda 2000 ms
  moverServoSuavemente(servo2, servo2.read(), coordenadas[1]);
  delay(2000);  // Aguarda 2000 ms
  moverServoSuavemente(servo3, servo3.read(), coordenadas[2]);
  delay(2000);  // Aguarda 2000 ms
  moverServoSuavemente(microServo1, microServo1.read(), coordenadas[3]);
  delay(2000);  // Aguarda 2000 ms
  moverServoSuavemente(microServo2, microServo2.read(), coordenadas[4]);
  delay(2000);  // Aguarda 2000 ms
  moverServoSuavemente(microServo3, microServo3.read(), coordenadas[5]);
  delay(2000);  // Aguarda 2000 ms
}

// Função para mover o servo suavemente
void moverServoSuavemente(Servo& servo, int posicao_inicial, int posicao_final, int passo = 1, int delay_time = 20) {
  if (posicao_inicial < posicao_final) {
    for (int pos = posicao_inicial; pos <= posicao_final; pos += passo) {
      servo.write(pos);
      delay(delay_time);
    }
  } else {
    for (int pos = posicao_inicial; pos >= posicao_final; pos -= passo) {
      servo.write(pos);
      delay(delay_time);
    }
  }
}

// Função para mover para a posição de descarte
void moverParaCoordenadasDescarte() {
  // Defina aqui a sequência de movimentos para despejar a peça
  // Exemplo:
  moverServoSuavemente(servo1, servo1.read(), 90);
  delay(1500);  // Aguarda 2000 ms
  moverServoSuavemente(servo2, servo2.read(), 90);
  delay(1500);  // Aguarda 2000 ms
  moverServoSuavemente(servo3, servo3.read(), 170);
  delay(1500);  // Aguarda 2000 ms
  moverServoSuavemente(microServo1, microServo1.read(), 170);
  delay(1500);  // Aguarda 2000 ms
  moverServoSuavemente(microServo2, microServo2.read(), 30);
  delay(1500);  // Aguarda 2000 ms
  moverServoSuavemente(microServo3, microServo3.read(), 80);
  delay(1500);  // Aguarda 2000 ms
}

// Movimento para pegar a peça 1 da primeira balança
void movimento11(){
  Serial.println("Pegando a peça 1.1...");

  moverServoSuavemente(servo1, servo1.read(), 25);
  delay(2000);  // Aguarda 2000 ms
  moverServoSuavemente(servo2, servo2.read(), 90);
  delay(2000);  // Aguarda 2000 ms
  moverServoSuavemente(microServo1, microServo1.read(), 170);
  delay(2000);  // Aguarda 2000 ms
  moverServoSuavemente(servo3, servo3.read(), 165);
  delay(2000);  // Aguarda 2000 ms
  moverServoSuavemente(microServo2, microServo2.read(), 45);
  delay(2000);  // Aguarda 2000 ms
  moverServoSuavemente(microServo3, microServo3.read(), 25);
  delay(2000);  // Aguarda 2000 ms
  moverServoSuavemente(servo3, servo3.read(), 140);
  delay(2000);  // Aguarda 2000 ms
  moverServoSuavemente(servo1, servo1.read(), 90);
  delay(2000);  // Aguarda 2000 ms
  moverServoSuavemente(servo3, servo3.read(), 170);
  delay(2000);  // Aguarda 2000 ms
  moverServoSuavemente(microServo3, microServo3.read(), 80);
  delay(2000);  // Aguarda 2000 ms

  // Sequência para despejar a peça
  moverParaCoordenadasDescarte();

  // Retorna à posição neutra após despejar a peça
  moverParaCoordenadas(posicao_neutra);
}

// Movimento para pegar a peça 2 da primeira balança
void movimento12(){
  Serial.println("Pegando a peça 1.2...");

  moverServoSuavemente(servo1, servo1.read(), 25);
  delay(2000);  // Aguarda 2000 ms
  moverServoSuavemente(servo2, servo2.read(), 100);
  delay(2000);  // Aguarda 2000 ms
  moverServoSuavemente(microServo1, microServo1.read(), 170);
  delay(2000);  // Aguarda 2000 ms
  moverServoSuavemente(servo3, servo3.read(), 160);
  delay(2000);  // Aguarda 2000 ms
  moverServoSuavemente(microServo2, microServo2.read(), 30);
  delay(2000);  // Aguarda 2000 ms
  moverServoSuavemente(microServo3, microServo3.read(), 18);
  delay(2000);  // Aguarda 2000 ms
  moverServoSuavemente(servo3, servo3.read(), 140);
  delay(2000);  // Aguarda 2000 ms

  // Sequência para despejar a peça
  moverParaCoordenadasDescarte();

  // Retorna à posição neutra após despejar a peça
  moverParaCoordenadas(posicao_neutra);
}

// Movimento para pegar a peça 1 da segunda balança
void movimento21(){
  Serial.println("Pegando a peça 2.1...");

  moverServoSuavemente(servo1, servo1.read(), 1);
  delay(2000);  // Aguarda 2000 ms
  moverServoSuavemente(servo2, servo2.read(), 90);
  delay(2000);  // Aguarda 2000 ms
  moverServoSuavemente(microServo1, microServo1.read(), 170);
  delay(2000);  // Aguarda 2000 ms
  moverServoSuavemente(servo3, servo3.read(), 165);
  delay(2000);  // Aguarda 2000 ms
  moverServoSuavemente(microServo2, microServo2.read(), 40);
  delay(2000);  // Aguarda 2000 ms
  moverServoSuavemente(microServo3, microServo3.read(), 25);
  delay(2000);  // Aguarda 2000 ms
  moverServoSuavemente(servo3, servo3.read(), 120);
  delay(2000);  // Aguarda 2000 ms

  // Sequência para despejar a peça
  moverParaCoordenadasDescarte();

  // Retorna à posição neutra após despejar a peça
  moverParaCoordenadas(posicao_neutra);
}

// Movimento para pegar a peça 2 da segunda balança
void movimento22(){
  Serial.println("Pegando a peça 2.2...");

  moverServoSuavemente(servo1, servo1.read(), 1);
  delay(2000);  // Aguarda 2000 ms
  moverServoSuavemente(servo2, servo2.read(), 105);
  delay(2000);  // Aguarda 2000 ms
  moverServoSuavemente(microServo1, microServo1.read(), 170);
  delay(2000);  // Aguarda 2000 ms
  moverServoSuavemente(servo3, servo3.read(), 160);
  delay(2000);  // Aguarda 2000 ms
  moverServoSuavemente(microServo2, microServo2.read(), 25);
  delay(2000);  // Aguarda 2000 ms
  moverServoSuavemente(microServo3, microServo3.read(), 25);
  delay(2000);  // Aguarda 2000 ms
  moverServoSuavemente(servo3, servo3.read(), 120);
  delay(2000);  // Aguarda 2000 ms

  // Sequência para despejar a peça
  moverParaCoordenadasDescarte();

  // Retorna à posição neutra após despejar a peça
  moverParaCoordenadas(posicao_neutra);
}

/*------------------------------------------------------------------------------ 
                    SEÇÃO DO MONITORAMENTO DAS BALANÇAS
------------------------------------------------------------------------------*/

// Constantes de calibração e configuração
#define CALIBVAL1 -401119   // Valor RAW de calibração para a balança 1 (com 25g)
#define CALIBVAL2 374849    // Valor RAW de calibração para a balança 2 (com 25g)
#define PESOBASE 25         // Peso base usado para calibração em gramas
#define BUTTON_TARA1 6      // Pino do botão de tara para a balança 1
#define BUTTON_TARA2 7      // Pino do botão de tara para a balança 2

// Pesos esperados e tolerâncias
#define PESO_ITEM1 32       // Peso médio do item 1 (em gramas)
#define TOLERANCIA1 6       // Tolerância para o item 1 (±3g)
#define PESO_ITEM2 25       // Peso médio do item 2 (em gramas)
#define TOLERANCIA2 6       // Tolerância para o item 2 (±3g)

// Pinos digitais para o HX711 da balança 1
const int LOADCELL1_DOUT_PIN = 2;
const int LOADCELL1_SCK_PIN = 3;

// Pinos digitais para o HX711 da balança 2
const int LOADCELL2_DOUT_PIN = 4;
const int LOADCELL2_SCK_PIN = 5;

// Valores iniciais de tara (ZEROVAL) para cada balança
long ZEROVAL1 = -391086;  // Ponto zero para a balança 1
long ZEROVAL2 = 386055;   // Ponto zero para a balança 2

// Buffers para leitura de dados de cada balança
long long BUFF1[50] = {0};  // Buffer para a balança 1
long long BUFF2[50] = {0};  // Buffer para a balança 2
byte BUFFL = 0;             // Tamanho do buffer

// Objetos HX711 para cada balança
HX711 scale1;
HX711 scale2;

// Variáveis para armazenar a quantidade de itens detectados
int quantidadeItem1 = 0;  // Quantidade de itens de 25g
int quantidadeItem2 = 0;  // Quantidade de itens de 32g

// Funções auxiliares (declarações)
long readRAW(HX711& scale, long long* BUFF);
float ConvertVal(long RAWVAL, long ZEROVAL, long CALIBVAL);
long calculateAverage(long long* BUFF);
void tare(HX711& scale, long& ZEROVAL, int buttonPin);
void identifyProduct(float peso, int balanca);

// Converte o valor RAW para gramas com base na calibração e no ponto de tara
float convertVal(long RAWVAL, long ZEROVAL, long CALIBVAL) {
  float unit = (float)PESOBASE / (float)(ZEROVAL - CALIBVAL);  // Unidade por grama
  return (float)(ZEROVAL - RAWVAL) * unit;  // Converte o RAW em peso
}

// Calcula a média do buffer de leituras
long calculateAverage(long long* BUFF) {
  long long ACC = 0;
  for (short i = 0; i < BUFFL; i++) {
    ACC += BUFF[i];
  }
  return (long)(ACC / BUFFL);  // Retorna a média
}

// Realiza a tara para a balança especificada
void tare(HX711& scale, long& ZEROVAL, int buttonPin) {
  Serial.print("Realizando tara na balança do botão ");
  Serial.println(buttonPin);
  while (!scale.is_ready());  // Aguarda que a balança esteja pronta
  ZEROVAL = scale.read();  // Define o novo ponto de tara
  Serial.println("Tara concluída!");
  Serial.print("Novo valor de tara (ZEROVAL): ");
  Serial.println(ZEROVAL);
}

// Identifica o tipo de item com base no peso e atualiza a quantidade
void identifyProduct(float peso, int balanca) {
  if (balanca == 1) {
    // Verifica se o peso está dentro da tolerância para o item 1
    if (peso >= PESO_ITEM1 - TOLERANCIA1 && peso <= PESO_ITEM1 + TOLERANCIA1) {
      quantidadeItem1 = round(peso / PESO_ITEM1);  // Calcula a quantidade
    } else if (peso < TOLERANCIA1) {  // Caso o peso seja muito baixo
      quantidadeItem1 = 0;  // Nenhum item detectado
    } else {
      Serial.println("Peso fora do padrão na balança 1!");
    }
  } else if (balanca == 2) {
    // Verifica se o peso está dentro da tolerância para o item 2
    if (peso >= PESO_ITEM2 - TOLERANCIA2 && peso <= PESO_ITEM2 + TOLERANCIA2) {
      quantidadeItem2 = round(peso / PESO_ITEM2);  // Calcula a quantidade
    } else if (peso < TOLERANCIA2) {  // Caso o peso seja muito baixo
      quantidadeItem2 = 0;  // Nenhum item detectado
    } else {
      Serial.println("Peso fora do padrão na balança 2!");
    }
  }
}

// Calcula peso 
void calculateWeight(long RAW1, long RAW2){
  // Coleta e processa leituras de ambas as balanças
  for (short z = 0; z < BUFFL; z++) {
    // Aguarda que ambas as balanças estejam prontas
    while (!scale1.is_ready() || !scale2.is_ready());

    // Leitura da balança 1
    long reading1 = scale1.read();
    for (short i = 0; i < BUFFL - 1; i++) {
      BUFF1[i] = BUFF1[i + 1];  // Desloca os valores no buffer
    }
    BUFF1[BUFFL - 1] = reading1;  // Adiciona a nova leitura

    // Leitura da balança 2
    long reading2 = scale2.read();
    for (short i = 0; i < BUFFL - 1; i++) {
      BUFF2[i] = BUFF2[i + 1];  // Desloca os valores no buffer
    }
    BUFF2[BUFFL - 1] = reading2;  // Adiciona a nova leitura
  }

  // Calcula a média das leituras RAW para suavizar os dados
  RAW1 = calculateAverage(BUFF1);
  RAW2 = calculateAverage(BUFF2);

  // Exibe os valores RAW para ambas as balanças
  Serial.print("RAW Balança 1: ");
  Serial.println(RAW1);  // Exibe o valor RAW da balança 1

  Serial.print("RAW Balança 2: ");
  Serial.println(RAW2);  // Exibe o valor RAW da balança 2

  // Converte os valores RAW para gramas
  float peso1 = convertVal(RAW1, ZEROVAL1, CALIBVAL1);
  float peso2 = convertVal(RAW2, ZEROVAL2, CALIBVAL2);

  // Exibe e identifica os itens para cada balança
  Serial.print("Balança 1 - Peso:");
  Serial.print(peso1);
  Serial.println("g");
  identifyProduct(peso1, 1);  // Identifica os itens da balança 1

  Serial.print("Balança 2 - Peso:");
  Serial.print(peso2);
  Serial.println("g");
  identifyProduct(peso2, 2);  // Identifica os itens da balança 2

  // Exibe as quantidades atuais de itens armazenados
  Serial.print("Quantidade de itens de 25g: ");
  Serial.println(quantidadeItem1);
  Serial.print("Quantidade de itens de 32g: ");
  Serial.println(quantidadeItem2);
}

/*------------------------------------------------------------------------------ 
                                    MAIN
------------------------------------------------------------------------------*/

void setup() {
  scale1.begin(LOADCELL1_DOUT_PIN, LOADCELL1_SCK_PIN);  // Inicializa a balança 1
  scale2.begin(LOADCELL2_DOUT_PIN, LOADCELL2_SCK_PIN);  // Inicializa a balança 2

  BUFFL = sizeof(BUFF1) / sizeof(BUFF1[0]);  // Calcula o tamanho do buffer

  pinMode(BUTTON_TARA1, INPUT_PULLUP);  // Configura o botão da balança 1
  pinMode(BUTTON_TARA2, INPUT_PULLUP);  // Configura o botão da balança 2

  Serial.println("Sistema iniciado.");  // Confirma que o sistema foi inicializado

  // Anexa os servos aos pinos digitais do Arduino
  servo1.attach(2);
  servo2.attach(3);
  servo3.attach(4);
  microServo1.attach(5);
  microServo2.attach(6);
  microServo3.attach(7);

  Serial.begin(9600);

  // Faz conexão WiFi
  wifiConnection();
  
  // Faz conexão socket com o servidor Python Storage.py
  socketConnection();

  // Move os servos para a posição neutra ao iniciar
  moverParaCoordenadas(posicao_neutra);

  // Exibe mensagem inicial no monitor serial das opções de
  Serial.println("Braço robótico inicializado na posição neutra.");
}

void loop() {
  /*------------------------------ 
    Botões para tarar as balanças 
  ------------------------------*/
  if (digitalRead(BUTTON_TARA1) == LOW) {
    tare(scale1, ZEROVAL1, BUTTON_TARA1);  // Realiza a tara na balança 1
  }
  if (digitalRead(BUTTON_TARA2) == LOW) {
    tare(scale2, ZEROVAL2, BUTTON_TARA2);  // Realiza a tara na balança 2
  }

  int command = 0
  
  long RAW1 = 0, RAW2 = 0;  // Leituras RAW das balanças

  command = receiveAndProcessData(sock)

  if (command != 0){
    calculateWeight(RAW1, RAW2)   

    if (command == 1){
      if (quantidadeItem1 == 2){
        // Sequência para pegar a peça 1.1
        movimento11();
      }else if (quantidadeItem1 == 1){
        // Sequência para pegar a peça 1.2
        movimento12();      
      }
    } else if (command == 2){
      if(quantidadeItem2 == 2){
        // Sequência para pegar a peça 2.1
        movimento21();
      }else if (quantidadeItem2 == 1){
        // Sequência para pegar a peça 2.2
        movimento22();
      }
    }

    lastQtyProd1 = quantidadeItem1
    lastQtyProd2 = quantidadeItem2

    delay(5000)

    calculateWeight(RAW1, RAW2)
    if(quantidadeItem1 < lastQtyProd1){
      createAndSendData(sock, lastQtyProd1 - quantidadeItem1, 1, 0);
    } else if (quantidadeItem1 > lastQtyProd1){
      createAndSendData(sock, quantidadeItem1 - lastQtyProd1, 1, 1);
    } else if(quantidadeItem2 < lastQtyProd2){
      createAndSendData(sock, lastQtyProd2 - quantidadeItem2, 2, 0);
    } else if(quantidadeItem2 > lastQtyProd2){
      createAndSendData(sock, quantidadeItem2 - lastQtyProd2, 2, 1);
    }

  }
  
}
