clear all;
close all;
clc;
more off;

freqAmostragem = 500000  % frequencia de amostragem (PS_8 no Arduino)
freqSinal = 20000        % frequencia do sinal
numeroMaximoDeAmostras = 500;

flag_grava_arquivos = 1;
flag_usa_senoide_teste = 1;

periodoAmostragem = 1.0/freqAmostragem
periodoSinal = 1.0/freqSinal
pontosPorCiclo = periodoSinal/periodoAmostragem;
numeroDeCiclos = floor(numeroMaximoDeAmostras/pontosPorCiclo)
vetorTempos = 0:periodoAmostragem:periodoSinal*numeroDeCiclos;

seno = sin(2*pi*freqSinal*vetorTempos);
cosseno = cos(2*pi*freqSinal*vetorTempos);

E = [seno' cosseno' ones(length(vetorTempos),1)];
piE = pinv(E); % pseudo-inversa de E - pode ser calculada offline!!!

% gravamos a piE no STM32... a multiplicação a seguir é feita dentro do STM32,
% e está aqui apenas como demonstração de funcionamento.
%
% o arquivos frame.txt contém um conjunto de dados coletados pelo arduino

% gravando piE no formato do codigo do STM32:
if flag_grava_arquivos
    fid = fopen ("piE.txt", "w");
    [nl, nc] = size(piE);
    fprintf(fid,"float piE[3][%d]{\n",length(vetorTempos));
    for linha = 1:nl
        fprintf(fid,"{");
        for coluna = 1:nc
            fprintf(fid,"%.6f, ",piE(linha,coluna));
        end
        fprintf(fid,"},\n");
    end
    fprintf(fid,"};\n");
    fclose(fid)
end

disp(['Usando ' num2str(length(vetorTempos)) ' pontos medidos:'])


dados = load('frame.txt');

% gerando pontos para teste:
if flag_usa_senoide_teste
    amplitude_teste = 1000;
    fase_teste = -1;
    offset_teste = 1500;
    dados = amplitude_teste*sin(2*pi*freqSinal*vetorTempos+fase_teste) +offset_teste;
    dados = round(dados');
    if flag_grava_arquivos
        fid = fopen ("seno.txt", "w");
        fprintf(fid,"uint16_t buffer2[] = {");
        for idx = 1:length(vetorTempos)
            fprintf(fid,"%d, ",dados(idx));
        end
        fprintf(fid,"};\n");
        fclose(fid)
    end
endif


figure(1); clf;
plot(dados)


disp('')
disp('*******************************************************************')
disp('Primeiro modo')

% conta a ser feita no arduino, com os dados coletados :
resp = piE*dados(1:length(vetorTempos));
R(1) = sqrt(resp(1)^2+resp(2)^2); %Amplitude
R(2) = atan(resp(2)/resp(1)); %Fase
R(3) = resp(3); %Offset
R(4) = length(vetorTempos); %N amostras
R

disp(['Amplitude do sinal: ' num2str(R(1))]);
disp(['Fase do sinal: ' num2str(R(2))]);
disp(['Offset do sinal: ' num2str(R(3))]);
disp(['Número de pontos utilizados: ' num2str(R(4)) ])


% este é um modo mais simples de fazer a mesma coisa no arduino ou STM32
disp('')
disp('*******************************************************************')
disp('Segundo modo')

piEvec = reshape(piE',1,length(vetorTempos)*3);
valorseno = 0;
valorcosseno = 0;
for i = 1:length(vetorTempos)
    valorseno = valorseno + dados(i)*(piEvec(i));
    valorcosseno = valorcosseno + dados(i)*(piEvec(i+length(vetorTempos)));
end
valorseno
valorcosseno
disp('Com o valorseno e valorcosseno, conseguimos calcular a amplitude e a fase:')
amplitude = sqrt(valorseno*valorseno + valorcosseno*valorcosseno)
fase = atan2(valorcosseno,valorseno)


% este é um modo mais simples de fazer a mesma coisa no arduino ou STM32
disp('')
disp('*******************************************************************')
disp('Terceiro modo')

valorseno = 0;
valorcosseno = 0;
for i = 1:length(vetorTempos)
    valorseno = valorseno + dados(i)*(piE(1,i));
    valorcosseno = valorcosseno + dados(i)*(piE(2,i));
end
valorseno
valorcosseno
disp('Com o valorseno e valorcosseno, conseguimos calcular a amplitude e a fase:')
amplitude = sqrt(valorseno*valorseno + valorcosseno*valorcosseno)
fase = atan2(valorcosseno,valorseno)


% verificando sinal estimado:
dados_est = amplitude*sin(2*pi*freqSinal*vetorTempos+fase) +mean(dados);
figure(1); hold on;
plot(dados_est,'r:');



