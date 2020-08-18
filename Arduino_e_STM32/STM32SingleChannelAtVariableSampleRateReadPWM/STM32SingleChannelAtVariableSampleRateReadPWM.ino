// Coleta dados analogicos do conversor AD;
// A taxa de amostragem eh controlada por um Timer via hardware
// O programa funciona da seguinte forma:
//   - inicia a coleta no setup();
//   - ao completar o buffer, pausa o timer e liga a flag_lido;
//   - transmite os dados coletados por Serial;
//   - aguarda 1000ms e reicinia o Timer e, com isso, a coleta.
// O programa supõe que existe uma onda PWM de frequência aproximada de 10kHz na porta A0.
//   - este PWM pode ser criado usando, por exemplo, um Arduino Nano e o programa ArduinoGeraPWM

#include <HardwareTimer.h>
#include <STM32ADC.h>

#define pinLED  PC13 // LED interno da placa

// flag_calcula_frequencia = 1: nao envia onda, apenas calcula frequencias
// flag_calcula_frequencia = 0: envia onda para Plotter Serial e nao calcula frequencias
int flag_calcula_frequencia = 1;


// Channels to be acquired.
// A0 (adc1 channel 1)
uint8 pins = 0;

#define maxSamples  500 // Numero de amostras a serem lidas de cada vez
uint16_t buffer[maxSamples];
uint8_t flag_lido;
uint16_t contador;

#define sampleFreqKhz       50
#define samplePeriodus      1000 / sampleFreqKhz
#define ticksPerSecond      2 * sampleFreqKhz * 1000 / maxSamples

int novo_sampleFreqKhz = sampleFreqKhz;

STM32ADC myADC(ADC1);

void TimerIRQ(void) { // O timer serve soh como trigger do ADC... 1 a cada amostra
}

void DmaIRQ(void) {                            // Completou o buffer...
  Timer3.pause();                              // pausa o timer
  flag_lido = 1;                               // liga a flag
  digitalWrite(pinLED, ! digitalRead(pinLED)); // altera estado do LED
}

void calcula_frequencia(){
  
  Serial.println("Frequencia_considerada_da_onda_(aprox):_10kHz");

  // Calculando frequencia de amostragem
  int Nciclos=0;
  int ultima_subida=0;
  int periodo=0;
  int soma_periodos=0;
  int nn;
  for (nn=0; nn<maxSamples-1; nn++){
      if(buffer[nn]<2000 && buffer[nn+1] >=2000){
          if(ultima_subida != 0){
              periodo = nn-ultima_subida;
              ultima_subida = nn;
              //Serial.print("Periodo da onda: ");
              //Serial.print(periodo);
              //Serial.println(" pontos");
              soma_periodos = soma_periodos + periodo;
              Nciclos += 1;
          }
          else{
              ultima_subida = nn;
          }
      }
  }
  float periodo_medio = (float)soma_periodos / (float)Nciclos;
  float freq_amostr_calc = periodo_medio*(float)10;
  Serial.print("Periodo_medio_(pontos):_"); Serial.println(periodo_medio);
  Serial.print("Numero_de_ciclos_completos_medidos:_"); Serial.print(Nciclos); Serial.print("_em_"); Serial.print(maxSamples); Serial.println("_pontos");
  Serial.print("Freq_amostr_calc_(aprox)[kHz]:"); Serial.print(freq_amostr_calc); Serial.print(", ");
  Serial.print("Freq_amostr_ajustada_[kHz]:"); Serial.println(novo_sampleFreqKhz);
  Serial.println("");

  //Serial.print(novo_sampleFreqKhz); Serial.print(" "); Serial.println(freq_amostr_calc); // envia dados para serem lidos pelo Ploter Serial
}

void setup() {

  pinMode(pinLED, OUTPUT);
  pinMode(pins, INPUT_ANALOG);

  Serial.begin(115200);

  flag_lido = 0;
  contador = 0;

  Timer3.setPeriod(samplePeriodus);
  Timer3.setMasterModeTrGo(TIMER_CR2_MMS_UPDATE);

  myADC.calibrate();
  myADC.setSampleRate(ADC_SMPR_1_5); // a cada trigger, a conversao eh feita o mais rapido possivel.
                                     // Talvez de para diminuir para melhorar acuracia da medida.
  myADC.setPins(&pins, 1);

  // Configurando DMA com:
  // - DMA_MINC_MODE: Auto-increment memory address
  // - DMA_CIRC_MODE: Circular mode
  // - DMA_TRNS_CMPLT: Interrupt on transfer completion
  myADC.setDMA(buffer, maxSamples, (DMA_MINC_MODE | DMA_CIRC_MODE | DMA_TRNS_CMPLT), DmaIRQ);
  
  myADC.setTrigger(ADC_EXT_EV_TIM3_TRGO); // ajusta Timer3 como trigger
  myADC.startConversion();
}

void loop() {

    if (flag_lido == 1) {
        // process data
        int nn;
        int ntotal = maxSamples;
        
        if (flag_calcula_frequencia == 0){ // envia onda para Plotter Serial e nao calcula frequencias
          for (nn=0; nn<ntotal; nn++){
              Serial.println(buffer[nn]); // Envia dados pela serial como ASCII, para ser visto com o 'Plotter Serial'
          }
        }
        else {
          // flag_calcula_frequencia = 1: nao envia onda, apenas calcula frequencias
          calcula_frequencia();
        }

        // altera a frequencia de amostragem
        novo_sampleFreqKhz += 1;
        if (novo_sampleFreqKhz>500){
            novo_sampleFreqKhz = 50;
            delay(1000); // aguarda X (ms)
        }
        
        int novo_samplePeriodus = 1000 / novo_sampleFreqKhz;
        Timer3.setPeriod(novo_samplePeriodus);
        Timer3.refresh();
        
        flag_lido=0;
        delay(10); // aguarda 10ms
        
        Timer3.resume(); // reinicia o timer e o ADC
    }


}
