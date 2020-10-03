// Baseado no código disponivel em:
// https://www.gameinstance.com/post/80/STM32-Oscilloscope-with-FFT-and-SD-export
// e
// https://github.com/gameinstance/STM32-Oscilloscope/
//
// Adaptado por Erick León.
//
// Realiza coleta em frequencias de amostragem entre 2,571MSps e 71,4kSps,
// conforme ajustado pela variavel 'time_base'.
// Envia os sinais pela serial, em ASCII, para visualização pelo PlotterSerial, 1x por segundo.
//

#include <STM32ADC.h>

static const uint8_t CHANNEL_1 = PB0;
//static const uint8_t CHANNEL_2 = PB1; // canal 2 nao usado

static const uint16_t BUFFER_SIZE = 500; // bytes
uint16_t data16[BUFFER_SIZE];
volatile static bool dma1_ch1_Active;

uint8_t time_base = 0; //  0      1      2      3      4      5      6      7     8
const uint8_t DT_PRE[]  = {0,     0,     0,     0,     0,     0,     0,     0,    1};    // adc prescaler
const uint8_t DT_SMPR[] = {0,     1,     2,     3,     4,     5,     6,     7,    7};    // adc cycles
const float DT_FS[]     = {2571,  1800,  1384,  878,   667,   529,   429,   143,  71.4}; // Sample Frequency


//////////////////////////////////////////////////////////////////////////////
// CONFIGURACAO DOS ADCs EM FUNCAO DO time_base:
// FS = 72 MHz / prescaler / (ADC_SMPR + 12.5)
// ex: FS = 72 MHz / 2 / (1.5 + 12.5) = 2.571 MSps !!!
void setADC() {
  switch (DT_PRE[time_base]) {
    case 0: rcc_set_prescaler(RCC_PRESCALER_ADC, RCC_ADCPRE_PCLK_DIV_2); break;
    case 1: rcc_set_prescaler(RCC_PRESCALER_ADC, RCC_ADCPRE_PCLK_DIV_4); break;
    case 2: rcc_set_prescaler(RCC_PRESCALER_ADC, RCC_ADCPRE_PCLK_DIV_6); break;
    case 3: rcc_set_prescaler(RCC_PRESCALER_ADC, RCC_ADCPRE_PCLK_DIV_8); break;
    default: rcc_set_prescaler(RCC_PRESCALER_ADC, RCC_ADCPRE_PCLK_DIV_8);
  }
  switch (DT_SMPR[time_base]) {
    case 0: adc_set_sample_rate(ADC1, ADC_SMPR_1_5); break;
    case 1: adc_set_sample_rate(ADC1, ADC_SMPR_7_5); break;
    case 2: adc_set_sample_rate(ADC1, ADC_SMPR_13_5); break;
    case 3: adc_set_sample_rate(ADC1, ADC_SMPR_28_5); break;
    case 4: adc_set_sample_rate(ADC1, ADC_SMPR_41_5); break;
    case 5: adc_set_sample_rate(ADC1, ADC_SMPR_55_5); break;
    case 6: adc_set_sample_rate(ADC1, ADC_SMPR_71_5); break;
    case 7: adc_set_sample_rate(ADC1, ADC_SMPR_239_5); break;
    default: adc_set_sample_rate(ADC1, ADC_SMPR_239_5);
  }
  adc_set_reg_seqlen(ADC1, 1); // Defines the total number of conversions in the regular channel conversion sequence.
  ADC1->regs->SQR3 = PIN_MAP[CHANNEL_1].adc_channel; // canal 1
  ADC1->regs->CR2 |= ADC_CR2_CONT; // | ADC_CR2_DMA; // Set continuous mode and DMA
  ADC1->regs->CR2 |= ADC_CR2_SWSTART;
}

//////////////////////////////////////////////////////////////////////////////
// Funcao executada por interrupcao quando a coleta termina
static void DMA1_CH1_Event() {
  dma1_ch1_Active = 0; // flag para perceber quando terminou a coleta
}

//////////////////////////////////////////////////////////////////////////////
// Habilita DMA para o ADC no CR2 (Control register 2)
// Parece fazer o mesmo que ADC_CR2_DMA comentado acima...
void adc_dma_enable(const adc_dev * dev) {
  bb_peri_set_bit(&dev->regs->CR2, ADC_CR2_DMA_BIT, 1); // Set a bit in an address in the peripheral bit-band region.
}

//////////////////////////////////////////////////////////////////////////////
// (http://docs.leaflabs.com/static.leaflabs.com/pub/leaflabs/maple-docs/latest/libmaple/api/dma.html)
// CONFIGURACAO DO DMA:
void setDMA(){
    // dma_init(dma_dev)
    // Initialize a DMA device. 
    // dma_dev * DMA1:
    //   DMA1 device. 
  dma_init(DMA1);
    // dma_attach_interrupt(dma_dev, dma_channel, function_handler)
    // Attach an interrupt to a DMA transfer. 
    // DMA channel (dma_channel enum):
    //   DMA_CH1 = Channel 1
  dma_attach_interrupt(DMA1, DMA_CH1, DMA1_CH1_Event);
    // Habilita DMA para o ADC no CR2 (Control register 2)
  adc_dma_enable(ADC1);
    // dma_setup_transfer(dma_dev, dma_channel, peripheral_address, peripheral_size, memory_address, memory_size, dma_mode)
    // Set up a DMA transfer. 
    // Source and destination transfer sizes (dma_xfer_size enum):
    //   DMA_SIZE_16BITS = 16-bit transfers
    // Flags for DMA transfer configuration (dma_mode_flags enum):
    //   DMA_MINC_MODE = Auto-increment memory address.
    //   DMA_TRNS_CMPLT = Interrupt on transfer completion.
  dma_setup_transfer(DMA1, DMA_CH1, &ADC1->regs->DR, DMA_SIZE_16BITS, data16, DMA_SIZE_16BITS, (DMA_MINC_MODE | DMA_TRNS_CMPLT));
    // dma_set_num_transfers(dma_dev, dma_channel, num_transfers)
    // Set the number of data to be transferred on a DMA channel. 
  dma_set_num_transfers(DMA1, DMA_CH1, BUFFER_SIZE);

}

//////////////////////////////////////////////////////////////////////////////
// Configuracao inicial
void setup() {
  adc_calibrate(ADC1); // Calibrate an ADC peripheral on STM32F1.
}

//////////////////////////////////////////////////////////////////////////////
// Loop:
void loop() {
  setADC();
  setDMA();

    // Inicia a coleta habilitando dma
  dma1_ch1_Active = 1; // flag para perceber quando terminou a coleta
  dma_enable(DMA1, DMA_CH1);                     // enable the DMA channel and start the transfer
    // Aguarda a coleta terminar e desabilita dma
  while (dma1_ch1_Active) {};                    // waiting for the DMA to complete
  dma_disable(DMA1, DMA_CH1);                    // end of DMA transfer

    // Envia dados pela serial para visualização no PlotterSerial
  int nn;
  for (nn=0; nn<BUFFER_SIZE-1; nn++){
    Serial.println(data16[nn]*3.3/4096.0); // converte em volts e envia
  }
  Serial.print("Coletas_realizadas_a_");
  Serial.print(DT_FS[time_base]);
  Serial.println("kSpS.");
  delay(1000); // aguarda 1s
}
