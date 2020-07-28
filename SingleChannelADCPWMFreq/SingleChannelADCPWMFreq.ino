// Coleta dados analogicos do conversor AD;
// A taxa de amostragem eh controlada por um Timer via hardware
// O programa funciona da seguinte forma:
//   - inicia a coleta no setup();
//   - ao completar o buffer, pausa o timer e liga a flag_lido;
//   - transmite os dados coletados por Serial;
//   - aguarda 1000ms e reicinia o Timer e, com isso, a coleta.
// Enquanto isso, cria um PWM (por software) na porta PB0 de frequencia aproximada sampleFreqkHz/50.
//   - se ligar a porta A0 na porta B0, deverao aparecer aprox 10 ciclos quadrados no Plotter Serial,
//     independentemente da frequencia usada...
// A cada coleta, calcula a frequencia de amostragem aproximada.

#include <HardwareTimer.h>
#include <STM32ADC.h>

#define pinLED  PC13 // LED interno da placa
#define pinOUT  PB0

// Channels to be acquired.
// A0 (adc1 channel 1)
uint8 pins = 0;

// flag_calcula_frequencia = 1: nao envia onda, apenas calcula frequencias
// flag_calcula_frequencia = 0: envia onda para Plotter Serial e nao calcula frequencias
int flag_calcula_frequencia = 1;

#define maxSamples  500 // Numero de amostras a serem lidas de cada vez
uint16_t buffer[maxSamples];
uint8_t flag_lido;
uint16_t contador;

#define sampleFreqKhz       350
#define samplePeriodus      1000 / sampleFreqKhz
#define ticksPerSecond      2 * sampleFreqKhz * 1000 / maxSamples

int novo_sampleFreqKhz = sampleFreqKhz;

STM32ADC myADC(ADC1);

void TimerIRQ(void) { // O timer serve soh como trigger do ADC... 1 a cada amostra
}

void DmaIRQ(void) { // Completou o buffer...
  Timer3.pause();
  flag_lido = 1;
  digitalWrite(pinLED, ! digitalRead(pinLED)); // altera estado do LED
}

void setup() {

  pinMode(pinLED, OUTPUT);
  pinMode(pinOUT, OUTPUT);
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
      
        flag_lido=0;      
        
        // process data
        int nn;
        int ntotal = maxSamples;

        if (flag_calcula_frequencia == 0){ // envia onda para Plotter Serial e nao calcula frequencias
            for (nn=0; nn<ntotal; nn++){
                Serial.println(buffer[nn]); // Envia dados pela serial como ASCII, para ser visto com o 'Plotter Serial'
            }
        }
        else { // flag_calcula_frequencia = 1: nao envia onda, apenas calcula frequencias

            Serial.print("Frequencia da onda (aprox): "); Serial.print(novo_sampleFreqKhz/50); Serial.println("kHz");
    
            // Calculando frequencia de amostragem
            int Nciclos=0;
            int ultima_subida=0;
            int periodo=0;
            int soma_periodos=0;
            for (nn=0; nn<ntotal-1; nn++){
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
            float periodo_medio = soma_periodos / Nciclos;
            Serial.print("Periodo medio (pontos): "); Serial.print(periodo_medio); Serial.println("pontos");
            Serial.print("Frequencia calculada (aprox): "); Serial.print(periodo_medio*sampleFreqKhz/50); Serial.println("kHz");
            Serial.print("Frequencia de amostragem ajustada: "); Serial.print(novo_sampleFreqKhz); Serial.println("kHz");
            Serial.println("");

            Serial.print(novo_sampleFreqKhz); Serial.print(" "); Serial.println(periodo_medio*sampleFreqKhz/50.0);
        }

        if (1==1){
            int novo_samplePeriodus = 1000 / novo_sampleFreqKhz;
            Timer3.setPeriod(novo_samplePeriodus);
            Timer3.refresh();

            
            novo_sampleFreqKhz += 1;
            if (novo_sampleFreqKhz>500){
                novo_sampleFreqKhz = 50;
                delay(2000); // aguarda X (ms)
            }
        }

        
        
        
        delay(10); // aguarda X (ms)
        
        Timer3.resume(); // reinicia o timer e o ADC
    }

    // Cria um PWM na porta PB0 de frequencia aproximada sampleFreqkHz/50
    if (contador == 50000/sampleFreqKhz){
      digitalWrite(pinOUT, LOW);
      contador = 0;
    }
    else if (contador == 25000/sampleFreqKhz){
      digitalWrite(pinOUT, HIGH);
    }
    contador += 1;
    
}
