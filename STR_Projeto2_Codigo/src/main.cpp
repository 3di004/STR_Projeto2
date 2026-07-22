#include <Arduino.h>
#include <iostream>

//Configurações do sistema
int capacidade_patio = 2;
SemaphoreHandle_t vagas_patio = xSemaphoreCreateBinary();
SemaphoreHandle_t trilho_compartilhado = xSemaphoreCreateCounting(capacidade_patio, 0);

int trem_produtor(int);

void setup() {
	Serial.begin(9600); //Iniciando comunicação serial para monitoramento do sistema
	pinMode(3, OUTPUT); //Saída digital:
	pinMode(4, OUTPUT); //Saída digital:
}

void loop() {
}

int trem_produtor(int linha) {
	return linha;
}