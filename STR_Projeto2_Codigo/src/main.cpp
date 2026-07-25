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

TaskHandle_t task1Handle, task2Handle, task3Handle; //Handles das tasks

//Configurações do sistema
int capacidade_patio = 2; //Quantidade de vagas no pátio.
int total_no_patio = 0; //Variável contadora do número de trens no pátio.
int randInt; //Número inteiro aleatório para descarregar o pátio.
int delayDebounce = 750; //ms
SemaphoreHandle_t vagas_patio = xSemaphoreCreateCounting(capacidade_patio, capacidade_patio); //Semáforo controlador das vagas do pátio, com a capacidade devida de 2 trens.
SemaphoreHandle_t trilho_compartilhado = xSemaphoreCreateCounting(1, 1); //Semáforo para o trilho compartilhado, de capacidade 1.

typedef struct {
	int id;
	SemaphoreHandle_t botao_iniciar_rota;
} ConfigTask_t;

ConfigTask_t configTask1, configTask2, configTask3;

void trem_produtor(void*); //Função a ser repassada como tasks ao FreeRTOS, representando cada trem das linhas

//Rotinas de Serviço de Interrupção de Hardware pelos botões
void IRAM_ATTR ISR_Bot1(){
	static TickType_t tempoAnt = 0; //Define uma variável estática para guardar o valor do tempo anterior na última chamada (realizar debounce).
	TickType_t tempoAtu = xTaskGetTickCountFromISR(); //Retornar o tempo em ticks do tempo decorrido desde o início da task.

	if (tempoAtu - tempoAnt >= pdMS_TO_TICKS(delayDebounce)){ //IF do debounce.
		BaseType_t taskMaiorPrioridade = pdFALSE; //Define variável para ver se a prioridade da task na CPU é maior que da task na RAM
		xSemaphoreGiveFromISR(configTask1.botao_iniciar_rota, &taskMaiorPrioridade); //aumenta em 1 o valor do semáforo que permite a task ser iniciada, alterando o taskMaiorPrioridade para pdTRUE caso a task parada pelo semáforo tiver prioridade maior que a task interrompida pelo botão.
		portYIELD_FROM_ISR(taskMaiorPrioridade); //Se taskMaiorPrioridade for pdTRUE, o scheduler pula direto para essa task.

		tempoAnt = tempoAtu; //Atualização de tempoAnt.
	}
}
//Idem para as outras rotinas de interrupção.
void IRAM_ATTR ISR_Bot2(){
	static TickType_t tempoAnt = 0;
	TickType_t tempoAtu = xTaskGetTickCountFromISR();

	if (tempoAtu - tempoAnt >= pdMS_TO_TICKS(delayDebounce)){
		BaseType_t taskMaiorPrioridade = pdFALSE;
		xSemaphoreGiveFromISR(configTask2.botao_iniciar_rota, &taskMaiorPrioridade);
		portYIELD_FROM_ISR(taskMaiorPrioridade);

		tempoAnt = tempoAtu;
	}
}
void IRAM_ATTR ISR_Bot3(){
	static TickType_t tempoAnt = 0;
	TickType_t tempoAtu = xTaskGetTickCountFromISR();

	if (tempoAtu - tempoAnt >= pdMS_TO_TICKS(delayDebounce)){
		BaseType_t taskMaiorPrioridade = pdFALSE;
		xSemaphoreGiveFromISR(configTask3.botao_iniciar_rota, &taskMaiorPrioridade);
		portYIELD_FROM_ISR(taskMaiorPrioridade);

		tempoAnt = tempoAtu;
	}
}

//Função modelo das tasks.
void trem_produtor(void *pvParameters){
	ConfigTask_t *config = (ConfigTask_t*)pvParameters;

	for (;;){
		if (xSemaphoreTake(config -> botao_iniciar_rota, portMAX_DELAY) == pdTRUE){
			Serial.print("[Linha "); Serial.print(config -> id); Serial.println("] Trem carregado e pronto para partir. Solicitando entrada no trilho compartilhado...");

			if (xSemaphoreTake(trilho_compartilhado, portMAX_DELAY) == pdTRUE){ //O if só é executado quando o trilho estiver vago.
				Serial.print("[Linha "); Serial.print(config -> id); Serial.println("] Trem entrou no trilho compartilhado.");
				digitalWrite(Led4, HIGH); //Led 4 ligado demonstrando que há um trem no trilho compartilhado

				vTaskDelay(pdMS_TO_TICKS(2000)); //Tempo de travessia do trilho.

				Serial.print("[Linha "); Serial.print(config -> id); Serial.println("] Trem chegou ao fim do trilho. Aguardando pela descarga no patio.");
				
				if (xSemaphoreTake(vagas_patio, portMAX_DELAY) == pdTRUE){ //No final do trilho compatilhado, o trem espera a entrada no pátio. Durante esse tempo, outro trem não pode entrar no trilho compartilhado.
					xSemaphoreGive(trilho_compartilhado); //O trem entra no pátio e o trilho compatilhado torna-se disponível.
					total_no_patio++;
					Serial.print("[Linha "); Serial.print(config -> id); Serial.println("] Trem entrou no patio. Descarregando...");
					Serial.print("[Patio] Vagas ocupadas: "); Serial.print(total_no_patio); Serial.print("/"); Serial.println(capacidade_patio);
					digitalWrite(Led4, LOW); //Led indicador de trem no trilho compartilhado desliga.
					digitalWrite(vetorLeds[(config -> id) - 1], HIGH); //O Led do trem da config -> id correspondente acende, indicando sua presença no pátio de descarga.

					vTaskDelay(pdMS_TO_TICKS(10000)); //Tempo que o trem passa descarregando.

					xSemaphoreGive(vagas_patio); //O semáforo de controle do pátio decresce de uma unidade.
					total_no_patio--; //O contador de trens no pátio descresce em uma unidade.
					Serial.print("[Linha "); Serial.print(config -> id); Serial.println("] Trem descarregou! Saindo do patio e voltando a mina.");
					Serial.print("[Patio] Vagas ocupadas: "); Serial.print(total_no_patio); Serial.print("/"); Serial.println(capacidade_patio);
					digitalWrite(vetorLeds[(config -> id) - 1], LOW);
				}
			}
		}
	}
}

//Setup do das portas e das tasks.
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

	attachInterrupt(digitalPinToInterrupt(Bot1), ISR_Bot1, FALLING);
	attachInterrupt(digitalPinToInterrupt(Bot2), ISR_Bot2, FALLING);
	attachInterrupt(digitalPinToInterrupt(Bot3), ISR_Bot3, FALLING);

	configTask1.botao_iniciar_rota = xSemaphoreCreateBinary();
	configTask2.botao_iniciar_rota = xSemaphoreCreateBinary();
	configTask3.botao_iniciar_rota = xSemaphoreCreateBinary();

	configTask1.id = 1;
	configTask2.id = 2;
	configTask3.id = 3;

	xTaskCreate(trem_produtor, "Task Trem 1", 2048, (void*)&configTask1, 1, &task1Handle);
	xTaskCreate(trem_produtor, "Task Trem 2", 2048, (void*)&configTask2, 1, &task2Handle);
	xTaskCreate(trem_produtor, "Task Trem 3", 2048, (void*)&configTask3, 1, &task3Handle);
}

//Loop: inutilizado.
void loop(){
	vTaskDelete(NULL);
}

