# stm32
Códigos de exemplo de uso do stm32.

## Programas:

### SingleChannelAtSampleRateSimpleDigOut

Programa para verificação da taxa de amostragem da coleta dados analogicos do conversor AD.

O próprio programa cria um PWM via software, coleta esse PWM (necessário ligar os pinos A0 e B0), e calcula a taxa de amostragem.

A taxa de amostragem é controlada por um Timer via hardware, que dispara o conversor AD.

O programa funciona da seguinte forma:

* inicia a coleta no setup();
* ao completar o buffer, pausa o timer e liga a flag_lido;
* transmite os dados coletados por Serial;
* aguarda 1000ms e reicinia o Timer e, com isso, a coleta.
* Enquanto isso, cria um PWM (por software) na porta PB0 de frequencia aproximada sampleFreqkHz/50.
* se ligar a porta A0 na porta B0, deverao aparecer aprox 10 ciclos quadrados no Plotter Serial, independentemente da frequencia usada.

### SingleChannelADCPWMFreq

Programa para verificação da taxa de amostragem da coleta dados analogicos do conversor AD.

O próprio programa cria um PWM via software, coleta esse PWM (necessário ligar os pinos A0 e B0), e calcula a taxa de amostragem.

A taxa de amostragem é controlada por um Timer via hardware, que dispara o conversor AD.

Neste programa, a taxa de amostragem varia automaticamente entre 50 e 500KHz, para que se possa observar o efeito em diferentes ajustes.

O programa funciona da seguinte forma:

* inicia a coleta no setup();
* ao completar o buffer, pausa o timer e liga a flag_lido;
* transmite os dados coletados por Serial;
* aguarda 1000ms e reicinia o Timer e, com isso, a coleta.
* Enquanto isso, cria um PWM (por software) na porta PB0 de frequencia aproximada sampleFreqkHz/50.
* se ligar a porta A0 na porta B0, deverao aparecer aprox 10 ciclos quadrados no Plotter Serial, independentemente da frequencia usada.
* o programa varia automaticamente a frequência de amostragem (de 1 em 1 entre 50 e 500kHz) e a frequência do PWM.

Alternativamente, pode-se ligar flag _flag\_calcula\_frequencia_, que faz com que o programa calcule a frequência amostrada em cada frequência ajustada e mostre os valores, que podem ver vistos no Monitor Serial ou no Ploter Serial.


