//O objetivo deste código é gerar um PWM pelo pino 9 para que o sinal seja lido em um STM32 para verificação
//da frequeência de aquisição

int PWMpin = 9; //Porta responsável por gerar o PWM (D9).
int sinal = 128; // duty cycle de 50%


void setup() {
  Serial.begin(9600); //Inicializa o SerialMonitor.
  pinMode(PWMpin, OUTPUT); //Define o PWMpin como saída, emissor da PWM.
  //analogWrite(PWMpin, sinal); //Envia o valor do PWM por hardware.
  Serial.println("Final da configuração"); //Marca o início dos dados.
}

void loop() {
  // PWM por software
  // ajuste 'fino' da frequência feita com osciloscópio
  // Frequência medida no Arduino Nano: aprox. 9945 Hz
  digitalWrite(PWMpin, HIGH);
  delayMicroseconds(47); // Approximately 50% duty cycle @ 10KHz
  digitalWrite(PWMpin, LOW);
  delayMicroseconds(47);
}
