# **Projeto 2 de Sistemas em Tempo Real**

> **Disciplina**: Sistemas em Tempo Real

> **Alunos**:
> - Artênio José Teofilo Correia
> - Edileudo da Silva Guedes Filho
> - José Vanilson de Brito Júnior

## **Vídeo demonstrativo de funcionamento**

YouTube: <youtube.com>

## **Descrição**

> - **Linguagem usada**: C
> - **Microcontrolador utilizado**: Esp 32 Devkit 1

### **> Visão Geral**

O sistema foi implementado fisicamente usando o microprocessador Esp32, com 4 botões e 4 leds. É mostrado pela figura a seguir o circuito na protoboard.

![](Figuras/Circuito_Fig01.jpeg)

Três botões representam o sistema mandando um trem da mina já carregado e um botão é usado para determinar o momento de descarregamento de um dos 3 trens de forma aleatorizada. Cada LED representa a vaga do trilho compartilhado e as três vagas do pátio de descarga.

#### **> Diagrama arquitetural das tarefas**

```mermaid
flowchart LR

classDef hardware fill: #9ad8f5, stroke: #0288d1, stroke-width: 2px, color: #000
classDef descarga fill: #ffb6ae, stroke: #ff4e18, stroke-width: 2px, color: #000
classDef isr fill: #acdfac, stroke: #15db1f, stroke-width: 2px, color: #000
classDef task fill: #fff88f, stroke: #fffb00, stroke-width: 2px, color: #000
classDef saida fill: #ff6b6b, stroke: #ff0000, stroke-width: 2px, color: #000

subgraph Entradas ["1. Hardware de Entrada, Botões"]
    Bot1["Mandar\n Trem 1"]:::hardware
    Bot2["Mandar\n Trem 2"]:::hardware
    Bot3["Mandar\n Trem 3"]:::hardware
    Bot4["Descarregar um\n dos trens"]:::descarga
end
subgraph ISRs ["2. Rotinas de Interrupção"]
    ISR1["ISR\n Bot1"]:::isr
    ISR2["ISR\n Bot2"]:::isr
    ISR3["ISR\n Bot3"]:::isr
    ISR4["ISR\n Bot4"]:::isr
end
subgraph Tasks ["3. Tarefas"]
    Task1["Tarefa 1"]:::task
    Task2["Tarefa 2"]:::task
    Task3["Tarefa 3"]:::task
end
subgraph Saídas ["4. Hardware de Saída, LEDs"]
    Led1["\n Vaga ocupada\n pelo trem 1"]:::saida
    Led2["Led\n Vaga ocupada\n pelo trem 2"]:::saida
    Led3["Led\n Vaga ocupada\n pelo trem 3"]:::saida
    Led4["Led\n Vaga no trilho\n compartilhado"]:::saida
end

Bot1 --> ISR1
Bot2 --> ISR2
Bot3 --> ISR3
Bot4 ==> ISR4

ISR1 --> Task1
ISR2 --> Task2
ISR3 --> Task3

ISR4 --> Task1
ISR4 --> Task2
ISR4 --> Task3

Task1 --> Led1
Task2 --> Led2
Task3 --> Led3

Task1 --> Led4
Task2 --> Led4
Task3 --> Led4

```


### **> Objetivos**

Retomar o projeto anterior, implementando-o em uma plataforma prática usando o FreeRTOS.

### **> Funcionamento do código**

O código utiliza o FreeRTOS para criar tarefas independentes que rodam de forma escalonada em um núcleo do processador do Esp32. Usando semáforos e mutexes, o sistema segue funcionando se auto gerenciando.