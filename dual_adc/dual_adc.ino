/* 
 * Exemplo de uso do ADC do STM32F103 em Dual Mode.
 *  
 * A taxa de amostragem é dada por:
 *  FS = 72 MHz / prescaler / (ADC_SMPR + 12.5)
 *  
 * Para PRE_SCALER==RCC_ADCPRE_PCLK_DIV_6 e ADC_SMPR==ADC_SMPR_1_5, a taxa de amostragem é:
 *  - Regular simultaneous mode: 857 kSps por canal, no caso de 2 canais
 *  - Fast interleaved mode: 1714 kSps
 *  
 * Para PRE_SCALER==RCC_ADCPRE_PCLK_DIV_2 e ADC_SMPR==ADC_SMPR_1_5, a taxa de amostragem é:
 *  - Regular simultaneous mode: 2.571 MSps por canal, no caso de 2 canais
 *  - Fast interleaved mode: 5.143 MSps
 *  
 * Obs: segundo nota do Reference Manual ([1], pág 215), não deve ser usado prescaler menor que 6 (72MHz/6 = 12MHz):
 * "The ADC input clock is generated from the PCLK2 clock divided by a prescaler and it must not exceed 14 MHz"
 * Provavelmente, a precisão da medida cai em frequências mais altas e cargas de alta impedância de saída devido aos 
 * capacitores do ADC (ADC do tipo "successive approximation analog-to-digital converter"). Ver [5], págs 6 e 33.
 *  
 * Escrito por: Erick León
 * 
 * Referências:
 * 
 * [1] STM32F10xxx Reference Manual (RM0008), disponível em www.st.com;
 * [2] Application note (AN3116), STM32's ADC modes and theis applications, disponível em www.st.com;
 * [3] Problems with regular simultaneous dual ADC conversion with DMA transfer, Spark Logic Forum,
 * disponível em https://sparklogic.ru/arduino-for-stm32/problems-with-regular-simultaneous-dual-adc.html
 * [4] https://github.com/rogerclarkmelbourne/Arduino_STM32/blob/master/STM32F1/libraries/STM32ADC/src/STM32ADC.cpp
 * [5] Application note (AN2834), How to get the best ADC accuracy in STM32 microcontrollers, disponível em www.st.com;

 * 
 */

#include "stm32_adc_dual_mode.h"


const float referenceVolts= 3.3;


#define CHANNELS_PER_ADC  1                      // number of channels for each ADC. Must match values in ADCx_Sequence array below
#define NUM_SAMPLES       500                    // number of samples for each ADCx. Each channel will be sampled NUM_SAMPLES/CHANNELS_PER_ADC
#define ADC_SMPR          ADC_SMPR_1_5           // when using dual mode, each pair of channels must have same rate. Here all channels have the same
#define PRE_SCALER        RCC_ADCPRE_PCLK_DIV_6  // Prescaler do ADC
#define FAST_INTERLEAVED  true                   // Fast Interleave Mode Flag. Para "dobrar" taxa de amostragem medindo o mesmo canal dos 2 ADCs.
                                                 // Se 'false', habilita "Regular simultaneous mode". Se 'true', habilita "Fast interleaved mode".


uint32 adcbuf[NUM_SAMPLES+1];  // buffer to hold samples, ADC1 16bit, ADC2 16 bit

// O STM32F103 possui 10 pinos do ADC disponíveis:
// pino A0 (PA0) -> 0 (ADC0)
// ...
// pino A7 (PA7) -> 7 (ADC7)
// pino B0 (PB0) -> 8 (ADC8)
// pino B1 (PB1) -> 9 (ADC9)
// Para "dobrar" taxa de amostragem (FAST_INTERLEAVED true), medir o mesmo canal dos 2 ADCs.
uint8 ADC1_Sequence[]={8,0,0,0,0,0};   // ADC1 channels sequence, left to right. Unused values must be 0. Note that these are ADC channels, not pins  
uint8 ADC2_Sequence[]={8,0,0,0,0,0};   // ADC2 channels sequence, left to right. Unused values must be 0


////////////////////////////////////////////////////////////////////////////////////
void setup() {
  Serial.begin(115200);
  set_adc_dual_channel(PRE_SCALER, ADC_SMPR, CHANNELS_PER_ADC, ADC1_Sequence, ADC2_Sequence, FAST_INTERLEAVED);  // initial ADC1 and ADC2 settings
}

////////////////////////////////////////////////////////////////////////////////////
void loop() {
  // medindo valores:
  start_convertion_dual_channel(adcbuf, NUM_SAMPLES);
  wait_convertion_dual_channel();

  // imprimindo valores lidos:
  for(int i=0;i<(NUM_SAMPLES);i++) {
    float volts= ((adcbuf[i] & 0xFFFF) / 4095.0)* referenceVolts;
    float voltss=  (((adcbuf[i] & 0xFFFF0000) >>16) / 4095.0)* referenceVolts;
    
    if(FAST_INTERLEAVED){ // Fast interleaved mode
      Serial.print("ADC:");
      Serial.println(voltss); //ADC2 é convertido primeiro... Ver [2], pág 10.
      Serial.print("ADC:");
      Serial.println(volts);
    }
    else{ // Regular simultaneous mode
      Serial.print("ADC1:");
      Serial.print(volts);
      Serial.print("\tADC2:");
      Serial.println(voltss);
    }
  }
  Serial.println();
 
  delay(1750);
}
