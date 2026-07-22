#include <Arduino.h>

#define botao1 4
#define botao2 16
#define botao3 17
#define led1 2
#define led2 5
#define led3 18
#define led4 19

//Configurações do sistema
int capacidade_patio = 2;
int total_no_patio = 0;
SemaphoreHandle_t vagas_patio = xSemaphoreCreateCounting(capacidade_patio, capacidade_patio); //Semáforo controlador das vagas do pátio, com a capacidade devida.
SemaphoreHandle_t trilho_compartilhado = xSemaphoreCreateBinary(); //Semáforo para o trilho compartilhado, de capacidade 1.

void trem_produtor(int);

void setup() {
	Serial.begin(9600); //Iniciando comunicação serial para monitoramento do sistema.
	pinMode(botao1, INPUT_PULLUP); //Entrada digital:
	pinMode(botao2, INPUT_PULLUP); //Entrada digital:
	pinMode(botao3, INPUT_PULLUP); //Entrada digital:
	pinMode(led1, OUTPUT); //Saída digital:
	pinMode(led2, OUTPUT); //Saída digital:
	pinMode(led3, OUTPUT); //Saída digital:
	pinMode(led4, OUTPUT); //Saída digital:
}

void loop() {
	if (!botao1){
		trem_produtor(1);
	} else if (!botao2){
		trem_produtor(2);
	} else if (!botao3){
		trem_produtor(3);
	}
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