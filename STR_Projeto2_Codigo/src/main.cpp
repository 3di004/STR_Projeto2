#include <Arduino.h>

#define Bot1 17	//Botão 1
#define Bot2 16	//Botão 2
#define Bot3 5	//Botão 3
#define Bot4 4	//Botão 4
#define Led1 23	//LED 1
#define Led2 22	//LED 2
#define Led3 21	//LED 3
#define Led4 19	//LED 4

byte vetorLeds[3] = {Led1, Led2, Led3}; //Vetor para acender os respectivos leds dentro das tasks criadas.

//Configuração dos botões
bool leitBot1 = true, leitBot2 = true, leitBot3 = true, leitBot4 = true;
bool leitBot1ant = true, leitBot2ant = true, leitBot3ant = true, leitBot4ant = true;

TaskHandle_t task1Handle, task2Handle, task3Handle, task4Handle; //Handles das tasks, para poder deletá-las ao fim do curso.

unsigned long tempoAtu = 0; //Tempo atual na contagem da função de debounce dos botões (ms).
unsigned long tempoAnt = 0; //Tempo anterior na contagem da função de debounce dos botões (ms).
unsigned long delayDebounce = 750; //Tempo entre acionamento dos botões (ms).

//Configurações do sistema
int capacidade_patio = 2; //Quantidade de vagas no pátio.
int total_no_patio = 0; //Variável contadora do número de trens no pátio.
int randInt; //Número inteiro aleatório para descarregar o pátio.
SemaphoreHandle_t vagas_patio = xSemaphoreCreateCounting(capacidade_patio, capacidade_patio); //Semáforo controlador das vagas do pátio, com a capacidade devida de 2 trens.
SemaphoreHandle_t trilho_compartilhado = xSemaphoreCreateCounting(1, 1); //Semáforo para o trilho compartilhado, de capacidade 1.

void trem_produtor(void*); //Função a ser repassada como tasks ao FreeRTOS, representando cada trem das linhas
void agente_descarregador(); //Agente que descarrega os trens de forma controlada pelo botão 4.

void setup(){
	Serial.begin(9600); //Iniciando comunicação serial para monitoramento do sistema.
	pinMode(Bot1, INPUT_PULLUP); //Entrada digital:
	pinMode(Bot2, INPUT_PULLUP); //Entrada digital:
	pinMode(Bot3, INPUT_PULLUP); //Entrada digital:
	pinMode(Bot4, INPUT_PULLUP); //Entrada digital:
	pinMode(Led1, OUTPUT); //Saída digital:
	pinMode(Led2, OUTPUT); //Saída digital:
	pinMode(Led3, OUTPUT); //Saída digital:
	pinMode(Led4, OUTPUT); //Saída digital:
}

void loop(){
	tempoAtu = millis(); //Contagem do tempo atual para função de debounce dos botões.
	//Leitura dos botões.
	leitBot1 = digitalRead(Bot1);
	leitBot2 = digitalRead(Bot2);
	leitBot3 = digitalRead(Bot3);
	leitBot4 = digitalRead(Bot4);

	if (tempoAtu - tempoAnt >= delayDebounce){ //Função de debounce (evitar execução repetida de funções).
		//Cada botão está em um IF que detecta subida de borda (evitar acionamento repetitivo enquanto o botão estiver pressionado) e se já existe uma task anterior em andamento.
		if (leitBot1 == false and leitBot1ant == true and !task1Handle){xTaskCreate(trem_produtor, "Task Trem 1", 2048, (void*)1, 1, &task1Handle); tempoAnt = tempoAtu;} 
		if (leitBot2 == false and leitBot2ant == true and !task2Handle){xTaskCreate(trem_produtor, "Task Trem 2", 2048, (void*)2, 1, &task2Handle); tempoAnt = tempoAtu;}
		if (leitBot3 == false and leitBot3ant == true and !task3Handle){xTaskCreate(trem_produtor, "Task Trem 3", 2048, (void*)3, 1, &task3Handle); tempoAnt = tempoAtu;}
		if (leitBot4 == false and leitBot4ant == true){agente_descarregador(); tempoAnt = tempoAtu;}
	}

	//Atribuição da leitura dos botões às variáveis de leitura anterior, prosseguindo para o próximo ciclo.
	leitBot1ant = leitBot1;
	leitBot2ant = leitBot2;
	leitBot3ant = leitBot3;
	leitBot4ant = leitBot4;
}

void trem_produtor(void *pvParameters){
	int linha = (int)(uintptr_t)pvParameters; //Definição do parâmetro único da função

	Serial.print("[Linha "); Serial.print(linha); Serial.println("] Trem carregado e pronto para partir. Solicitando entrada no trilho compartilhado...");

	if (xSemaphoreTake(trilho_compartilhado, portMAX_DELAY) == pdTRUE){ //O if só é executado quando o trilho estiver vago.
		Serial.print("[Linha "); Serial.print(linha); Serial.println("] Trem entrou no trilho compartilhado.");
		digitalWrite(Led4, HIGH); //Led 4 ligado demonstrando que há um trem no trilho compartilhado

		vTaskDelay(pdMS_TO_TICKS(2000)); //Tempo de travessia do trilho.

		Serial.print("[Linha "); Serial.print(linha); Serial.println("] Trem chegou ao fim do trilho. Aguardando pela descarga no patio.");
		
		if (xSemaphoreTake(vagas_patio, portMAX_DELAY) == pdTRUE){ //No final do trilho compatilhado, o trem espera a entrada no pátio. Durante esse tempo, outro trem não pode entrar no trilho compartilhado.
			xSemaphoreGive(trilho_compartilhado); //O trem entra no pátio e o trilho compatilhado torna-se disponível.
			total_no_patio++;
			Serial.print("[Linha "); Serial.print(linha); Serial.print("] Trem entrou no patio. Descarregando... (Vagas ocupadas: "); Serial.print(total_no_patio); Serial.print("/"); Serial.println(capacidade_patio); Serial.print(")");
			digitalWrite(Led4, LOW); //Led indicador de trem no trilho compartilhado desliga.
			digitalWrite(vetorLeds[linha - 1], HIGH); //O Led do trem da linha correspondente acende, indicando sua presença no pátio de descarga.

			vTaskDelay(pdMS_TO_TICKS(10000)); //Tempo que o trem passa descarregando.

			xSemaphoreGive(vagas_patio); //O semáforo de controle do pátio decresce de uma unidade.
			total_no_patio--; //O contador de trens no pátio descresce em uma unidade.
			Serial.print("[Linha "); Serial.print(linha); Serial.println("] Trem descarregou! Saindo do patio e voltando a mina. (Vagas ocupadas: "); Serial.print(total_no_patio); Serial.print("/"); Serial.println(capacidade_patio); Serial.print(")");
			digitalWrite(vetorLeds[linha - 1], LOW);

			switch (linha){	//Switch para definir a Handle da tarefa respectiva como NULL, indicando seu apagamento do sistema.
				case 1: task1Handle = NULL; break;
				case 2: task2Handle = NULL; break;
				case 3: task3Handle = NULL; break;
			}
			vTaskDelete(NULL); //Após a task é concluída, ela é então apagada.
		}
	}
}

void agente_descarregador(){
	if (task1Handle != NULL){
		vTaskDelete(task1Handle); task1Handle = NULL;
		Serial.println("[Linha 1] Trem descarregou! Saindo do patio e voltando a mina.");
	} else if (task2Handle != NULL){
		vTaskDelete(task2Handle); task2Handle = NULL;
		Serial.println("[Linha 2] Trem descarregou! Saindo do patio e voltando a mina.");
	} else if (task3Handle != NULL){
		vTaskDelete(task3Handle); task3Handle = NULL;
		Serial.println("[Linha 3] Trem descarregou! Saindo do patio e voltando a mina.");
	}
}