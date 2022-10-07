#include <stdio.h>

enum status { PARADO, BLOQUEADO, EXECUTANDO_CPU, EXECUTANDO_IO };
enum io { DISCO = 3, MAGNETICA = 5, IMPRESSORA = 7 };

#define QUANTUM 5

/*
Criar um contrutor chamado TipoIO que conterá:
. String Tipo ("DISCO", "MAGNETICA", "IMPRESSORA")
. booleano Prioridade (0 - baixa, 1 - alta)
. int Duracao (em unidade de tempo)
*/
typedef struct
{
    enum io tipo;
    int tempoDeEntrada;
} TipoIO;

/*
Criar um contrutor chamado PROCESSO que conterá:
. PID
. Prioridade (quanto menor maior a prioridade)
. Tempo de chegada *
. Tempo do processo
. PPID (não sei o que é isso)
. Status (PARADO, BLOQUEADO, EXECUTANDO CPU, EXECUTANDO IO)
*/
typedef struct 
{
    int pid;
    int prioridade;
    int tempoChegada;
    int tempoTotalDeProcesso;
    enum status situacao;
    int tempoProcessado;
    TipoIO entradaEhSaida[10];
} processo;

/*
Criar um contrutor chamado ESCALONADOR que conterá:
. Fila de alta prioridade
. Fila de baixa prioridade
. Fila TipoIO
*/
typedef struct
{
    processo altaPrioridade[100];
    processo baixaPrioridade[100];
    processo entradaSaida[100];
    processo espera[100];
} escalonador;

void main() {
    /*
    Menu:
     1 - Introdução
     2 - Criar processos quantidade de 0 - 100
     3 - Executar os processos
     4 - Parar processos
     5 - Detalhes
     6 - Zerar**
    */

   /*
    no loop principal 
   */
}