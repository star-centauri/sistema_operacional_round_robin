#include <stdio.h>

enum status { PARADO, BLOQUEADO, EXECUTANDO_CPU, EXECUTANDO_IO };
enum io { DISCO, MAGNETICA, IMPRESSORA };
/*
Criar um contrutor chamado PROCESSO que conterá:
. PID
. Prioridade (quanto menor maior a prioridade)
. Tempo de chegada *
. Tempo do processo
. PPID (não sei o que é isso)
. Status (PARADO, BLOQUEADO, EXECUTANDO CPU, EXECUTANDO IO)
*/
struct processo
{
    char pid[6];
    int prioridade;
    int tempoChegada;
    int tempoProcesso;
    enum status situacao;
};

/*
Criar um contrutor chamado TipoIO que conterá:
. String Tipo ("DISCO", "MAGNETICA", "IMPRESSORA")
. booleano Prioridade (0 - baixa, 1 - alta)
. int Duracao (em unidade de tempo)
*/
struct TipoIO
{
    enum io tipo;
    int prioridade;
    int duracao;
};

/*
Criar um contrutor chamado ESCALONADOR que conterá:
. Fila de alta prioridade
. Fila de baixa prioridade
. Fila TipoIO
*/
struct escalonador
{
    struct processo altaPrioridade[100];
    struct processo baixaPrioridade[100];
    struct TipoIO entradaSaida[3];
    
};

void main() {
    /*
    Menu:
     1 - Introdução
     2 - Criar processos quantidade de 0 - 100
     3 - Executar os processos
     4 - Parar processos
     5 - Detalhes
     6 - Zerar
    */
}