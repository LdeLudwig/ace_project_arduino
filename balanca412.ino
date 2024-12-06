#include "HX711.h"

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
long ReadRAW(HX711& scale, long long* BUFF);
float ConvertVal(long RAWVAL, long ZEROVAL, long CALIBVAL);
long CalculateAverage(long long* BUFF);
void Tara(HX711& scale, long& ZEROVAL, int buttonPin);
void IdentificarPeca(float peso, int balanca);

void setup() {
  // Configuração inicial
  Serial.begin(115200);  // Inicializa a comunicação serial
  scale1.begin(LOADCELL1_DOUT_PIN, LOADCELL1_SCK_PIN);  // Inicializa a balança 1
  scale2.begin(LOADCELL2_DOUT_PIN, LOADCELL2_SCK_PIN);  // Inicializa a balança 2

  BUFFL = sizeof(BUFF1) / sizeof(BUFF1[0]);  // Calcula o tamanho do buffer

  pinMode(BUTTON_TARA1, INPUT_PULLUP);  // Configura o botão da balança 1
  pinMode(BUTTON_TARA2, INPUT_PULLUP);  // Configura o botão da balança 2

  Serial.println("Sistema iniciado.");  // Confirma que o sistema foi inicializado
}

void loop() {
  // Verifica se os botões foram pressionados para realizar a tara
  if (digitalRead(BUTTON_TARA1) == LOW) {
    Tara(scale1, ZEROVAL1, BUTTON_TARA1);  // Realiza a tara na balança 1
  }
  if (digitalRead(BUTTON_TARA2) == LOW) {
    Tara(scale2, ZEROVAL2, BUTTON_TARA2);  // Realiza a tara na balança 2
  }

  long RAW1 = 0, RAW2 = 0;  // Leituras RAW das balanças

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
  RAW1 = CalculateAverage(BUFF1);
  RAW2 = CalculateAverage(BUFF2);

  // Exibe os valores RAW para ambas as balanças
  Serial.print("RAW Balança 1: ");
  Serial.println(RAW1);  // Exibe o valor RAW da balança 1

  Serial.print("RAW Balança 2: ");
  Serial.println(RAW2);  // Exibe o valor RAW da balança 2

  // Converte os valores RAW para gramas
  float peso1 = ConvertVal(RAW1, ZEROVAL1, CALIBVAL1);
  float peso2 = ConvertVal(RAW2, ZEROVAL2, CALIBVAL2);

  // Exibe e identifica os itens para cada balança
  Serial.print("Balança 1 - Peso:");
  Serial.print(peso1);
  Serial.println("g");
  IdentificarPeca(peso1, 1);  // Identifica os itens da balança 1

  Serial.print("Balança 2 - Peso:");
  Serial.print(peso2);
  Serial.println("g");
  IdentificarPeca(peso2, 2);  // Identifica os itens da balança 2

  // Exibe as quantidades atuais de itens armazenados
  Serial.print("Quantidade de itens de 25g: ");
  Serial.println(quantidadeItem1);
  Serial.print("Quantidade de itens de 32g: ");
  Serial.println(quantidadeItem2);
}

// Converte o valor RAW para gramas com base na calibração e no ponto de tara
float ConvertVal(long RAWVAL, long ZEROVAL, long CALIBVAL) {
  float unit = (float)PESOBASE / (float)(ZEROVAL - CALIBVAL);  // Unidade por grama
  return (float)(ZEROVAL - RAWVAL) * unit;  // Converte o RAW em peso
}

// Calcula a média do buffer de leituras
long CalculateAverage(long long* BUFF) {
  long long ACC = 0;
  for (short i = 0; i < BUFFL; i++) {
    ACC += BUFF[i];
  }
  return (long)(ACC / BUFFL);  // Retorna a média
}

// Realiza a tara para a balança especificada
void Tara(HX711& scale, long& ZEROVAL, int buttonPin) {
  Serial.print("Realizando tara na balança do botão ");
  Serial.println(buttonPin);
  while (!scale.is_ready());  // Aguarda que a balança esteja pronta
  ZEROVAL = scale.read();  // Define o novo ponto de tara
  Serial.println("Tara concluída!");
  Serial.print("Novo valor de tara (ZEROVAL): ");
  Serial.println(ZEROVAL);
}

// Identifica o tipo de item com base no peso e atualiza a quantidade
void IdentificarPeca(float peso, int balanca) {
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