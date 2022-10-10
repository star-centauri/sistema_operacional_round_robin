#include <stdio.h>
#include <stdlib.h>

enum status { PARADO, BLOQUEADO, EXECUTANDO_CPU, EXECUTANDO_IO };
enum io { DISCO = 3, MAGNETICA = 5, IMPRESSORA = 7 };

#define QUANTUM 5
#define QTD_PROCESSOS 100

/*
Criar um contrutor chamado TipoIO que conterá:
. enum io Tipo = guarda o tipo de IO e junto seu tempo de execução "DISCO", "MAGNETICA", "IMPRESSORA"
. int tempoDeEntrada = o momento que o processo irá entrar na fila de IO
*/
typedef struct
{
    enum io tipo;
    int tempoDeEntrada;
} TipoIO;

/*
Criar um contrutor chamado PROCESSO que conterá:
. int PID = identificador do processo
. int Prioridade = quanto menor maior a prioridade
. int tempoChegada = tempo que o processo está pronto para execução
. int tempoTotalDeProcesso = tempo de execução total do processo
. enum status situacao = entender em qual situação o processo se encontra em um determinado instante
. int tempoProcessado = tempo que o processo já executou
. enum TipoIO entradaEhSaida[10] = as entradas ao IO, quais e em que tempo (caso tenha) 
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
Estrutura de fila
. processo rows = guarda a lista de processos
. int end = o indice da ultima posicao vazia da fila
*/
typedef struct
{
    processo rows[QTD_PROCESSOS];
    int end;
} fifo;


/*
Criar um contrutor chamado ESCALONADOR que conterá:
. Fila de alta prioridade
. Fila de baixa prioridade
. Fila entradaSaida
. Fila de espera
*/
typedef struct
{
    fifo altaPrioridade[QTD_PROCESSOS];
    fifo baixaPrioridade[QTD_PROCESSOS];
    fifo entradaSaida[QTD_PROCESSOS];
    fifo espera[QTD_PROCESSOS];
} escalonador;

fifo create();
void enqueue(processo dado, fifo *f);
processo dequeue(fifo *f);
void printProcessos(fifo *f);

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

   fifo teste = create();

   processo processo1;
   processo1.pid = 1;
   processo1.prioridade = 10;

    enqueue(processo1, &teste);

   processo processo2;
   processo2.pid = 2;
   processo2.prioridade = 20;

    enqueue(processo2, &teste);
    printProcessos(&teste);

   processo processo3;
   processo3.pid = 3;
   processo3.prioridade = 30;

   processo removido2 = dequeue(&teste);

   printProcessos(&teste);
   enqueue(processo3, &teste);
   printProcessos(&teste);
}

fifo create() {
    fifo fila;
    fila.end = 0;

    return fila;
}

void enqueue(processo dado, fifo *f) {
    if ( f->end == QTD_PROCESSOS ) {
        printf("FILA ESTÁ CHEIA!\n");
    }
    else {
        f->rows[f->end] = dado;
        f->end++;
    }
}

processo dequeue(fifo *f) {
    processo backup;

    if ( f->end == 0 ) {
        printf("FILA VAZIA!\n");
        return;
    }
    else {
        backup = f->rows[0];

        for (int i = 0; i < f->end; i++)
        {
            f->rows[i] = f->rows[i+1];
        }
        
        f->end--;
        return backup;
    }
}

void printProcessos(fifo *f) {
    printf("Lendo processos na fila:\n");
    for (int i = 0; i < f->end; i++)
    {
        printf("PID %d\n", f->rows[i].pid);
    }   
    printf("-----------------------------\n");
}