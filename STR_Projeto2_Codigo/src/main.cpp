#include <Arduino.h>

#define B1 4	//Botão 1
#define B2 5	//Botão 2
#define B3 16	//Botão 3
#define L1 17	//LED 1
#define L2 18	//LED 2
#define L3 19	//LED 3
#define L4 21	//LED 4

//Configuração dos botões
bool leitB1;
bool leitB2;
bool leitB3;
bool leitB1ant = true;
bool leitB2ant = true;
bool leitB3ant = true;

//Configurações do sistema
int capacidade_patio = 2;
int total_no_patio = 0;
SemaphoreHandle_t vagas_patio = xSemaphoreCreateCounting(capacidade_patio, capacidade_patio); //Semáforo controlador das vagas do pátio, com a capacidade devida.
SemaphoreHandle_t trilho_compartilhado = xSemaphoreCreateBinary(); //Semáforo para o trilho compartilhado, de capacidade 1.

void trem_produtor(int);

void setup() {
	Serial.begin(9600); //Iniciando comunicação serial para monitoramento do sistema.
	pinMode(B1, INPUT_PULLUP); //Entrada digital:
	pinMode(B2, INPUT_PULLUP); //Entrada digital:
	pinMode(B3, INPUT_PULLUP); //Entrada digital:
	pinMode(L1, OUTPUT); //Saída digital:
	pinMode(L2, OUTPUT); //Saída digital:
	pinMode(L3, OUTPUT); //Saída digital:
	pinMode(L4, OUTPUT); //Saída digital:
}

void loop() {
	leitB1 = digitalRead(B1);
	if (!leitB1){
		//trem_produtor(1);
		digitalWrite(L1, HIGH);
	} else {
		digitalWrite(L1, LOW);
	}
	//leitB1ant = leitB1;
}

void trem_produtor(int linha) {
	Serial.print("[Linha "); Serial.print(linha); Serial.println("] Trem carregado e pronto para partir. Solicitando entrada no trilho compartilhado...");

	if (xSemaphoreTake(trilho_compartilhado, portMAX_DELAY) == pdTRUE){ //O if só é executado quando o trilho estiver vago.
		Serial.print("[Linha "); Serial.print(linha); Serial.println("] Trem entrou no trilho compartilhado.");

		vTaskDelay(pdMS_TO_TICKS(2500)); //Tempo de travessia do trilho.

		Serial.print("[Linha "); Serial.print(linha); Serial.println("] Trem chegou ao fim do trilho. Aguardando pela descarga no pátio.");
		
		if (xSemaphoreTake(vagas_patio, portMAX_DELAY) == pdTRUE){ //No final do trilho compatilhado, o trem espera a entrada no pátio. Durante esse tempo, outro trem não pode entrar no trilho compartilhado.
			xSemaphoreGive(trilho_compartilhado); //O trem entra no pátio e o trilho compatilhado torna-se disponível
			total_no_patio++;
			Serial.print("[Linha "); Serial.print(linha); Serial.println("] Trem entrou no pátio. Descarregando...");

			vTaskDelay(pdMS_TO_TICKS(3000)); //Tempo que o trem passa descarregando.

			xSemaphoreGive(vagas_patio);
			total_no_patio--;
			Serial.print("[Linha "); Serial.print(linha); Serial.println("] Trem descarregou! Saindo do pátio e voltando à mina.");
		}
	}
}