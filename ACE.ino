#include <Servo.h>

// Define os servos
Servo servo1;
Servo servo2;
Servo servo3;
Servo microServo1;
Servo microServo2;
Servo microServo3;

void setup() {
  // Anexa os servos aos pinos digitais do Arduino
  servo1.attach(2);
  servo2.attach(3);
  servo3.attach(4);
  microServo1.attach(5);
  microServo2.attach(6);
  microServo3.attach(7);

  Serial.begin(9600);

  // Move os servos para a posição neutra ao iniciar
  moverParaCoordenadas(posicao_neutra);

  // Exibe mensagem inicial no monitor serial
  Serial.println("Braço robótico inicializado na posição neutra.");
  Serial.println("Digite '1' para pegar a peça 1.1.");
  Serial.println("Digite '2' para pegar a peça 1.2.");
  Serial.println("Digite '3' para pegar a peça 2.1.");
  Serial.println("Digite '4' para pegar a peça 2.2.");
}

// Posições neutras para os servos
int posicao_neutra[6] = {90, 90, 90, 90, 90, 80};

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

void loop() {
  // Espera por uma entrada do usuário
  if (Serial.available() > 0) {
    int comando = Serial.parseInt();
    if (comando == 1) {
      // Sequência para pegar a peça 1.1
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

      Serial.println("Peça 1.1 descartada. Digite '1' para pegar novamente.");
      Serial.println("Digite '2' para pegar a peça 1.2.");
      Serial.println("Digite '3' para pegar a peça 2.1.");
      Serial.println("Digite '4' para pegar a peça 2.2.");
    } else if (comando == 2) {
      // Sequência para pegar a peça 1.2
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

      Serial.println("Peça 1.2 descartada. Digite '1' para pegar a peça 1.1.");
      Serial.println("Digite '2' para pegar a peça 1.2 novamente.");
      Serial.println("Digite '3' para pegar a peça 2.1.");
      Serial.println("Digite '4' para pegar a peça 2.2.");
    } else if (comando == 3) {
      // Sequência para pegar a peça 2.1
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

      Serial.println("Peça 2.1 descartada. Digite '1' para pegar a peça 1.1.");
      Serial.println("Digite '2' para pegar a peça 1.2.");
      Serial.println("Digite '3' para pegar a peça 2.1 novamente.");
      Serial.println("Digite '4' para pegar a peça 2.2.");
    } else if (comando == 4) {
      // Sequência para pegar a peça 2.2
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

      Serial.println("Peça 2.2 descartada. Digite '1' para pegar a peça 1.1.");
      Serial.println("Digite '2' para pegar a peça 1.2.");
      Serial.println("Digite '3' para pegar a peça 2.1.");
      Serial.println("Digite '4' para pegar a peça 2.2 novamente.");
    }
  }
}


