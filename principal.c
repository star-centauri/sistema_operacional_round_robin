#include <stdio.h>
#include <stdlib.h>

enum status { PARADO, BLOQUEADO, EXECUTANDO_CPU, EXECUTANDO_IO };
enum io { DISCO = 3, MAGNETICA = 5, IMPRESSORA = 7 };

#define QUANTUM 5
#define QTD_PROCESSOS 20

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
. lista de espera: todos os processos
*/
typedef struct
{
    fifo altaPrioridade;
    fifo baixaPrioridade;
    fifo entradaSaida;
    processo espera[QTD_PROCESSOS];
} escalonador;

fifo create();
void enqueue(processo dado, fifo *f);
processo dequeue(fifo *f);
void printProcessos(fifo *f);
void runProcesses(escalonador *filas);
processo getProcessosEspera(processo *espera[QTD_PROCESSOS], int elapsedTime);

void main() {
    /*
    Menu:
     1 - Introdução
     2 - Criar processos quantidade de 0 - 20
     3 - Executar os processos
     4 - Parar processos
     5 - Detalhes
     6 - Zerar**
    */
   escalonador filas;
   filas.altaPrioridade = create();
   filas.baixaPrioridade = create();
   filas.entradaSaida = create();

    runProcesses(&filas);
}

// =============== BEGIN ALGORITMO ROUND ROBIN =============== //
void runProcesses(escalonador *filas) {
    int run = 1; // MANTER EXECUÇÃO DO ALGORITMO ATÉ SE SOLICITADO PARAR
    int elapsedTime = 0; // SIMULA O TEMPO CORRIDO DENTRO DO LOOP (VAI SER IMPORTANTE PARA O TEMPO DE CHEGADA)

    // LOOP DE TODA EXECUÇÃO DO ALGORITMO
    while (run)
    {
        int countQuantumAlta;
        // VERIFIFCO SE TEM ALGUM PROCESSO NOVO PARA ADICIONAR A LISTA DE PRIORIDADES
        processo pronto = getProcessosEspera(&filas->espera, elapsedTime);
        filas->altaPrioridade.rows[filas->altaPrioridade.end] = pronto;
        
        // LOOP EXECUTA ALTA PRIORIDADE (AINDA SEM VERIFICACAO IO)
        processo executa = dequeue(&filas->altaPrioridade);
        for ( countQuantumAlta = 0; countQuantumAlta < QUANTUM; countQuantumAlta++ )
        {
            // PARAR LOOP CASO JA TENHA FEITO O PROCESSO POR COMPLETO
            if ( executa.tempoProcessado == executa.tempoTotalDeProcesso ) { 
                break;
            }

            executa.tempoProcessado++;
        }
        // PROCESSO NAO FINALIZOU NO PRIMEIRO QUANTUM, VAI PARA ESPERA
        if ( executa.tempoProcessado != executa.tempoTotalDeProcesso ) {
            filas->entradaSaida.rows[filas->entradaSaida.end] = executa;
        }
        
        elapsedTime++;
    }
    
}

/// @brief FAZ A BUSCAR PELOS PROCESSOS NOVOS QUE ESTÃO PRONTO PARA O ESCALONADOR
/// @param espera 
/// @param elapsedTime 
/// @return processo 
processo getProcessosEspera(processo *espera[QTD_PROCESSOS], int elapsedTime) {
    processo pronto;
    for (int i = 0; i < QTD_PROCESSOS; i++)
    {
        if (espera[i]->tempoChegada <= elapsedTime) {
            pronto = *espera[i];
            
            for (int j = i; i < QTD_PROCESSOS - 1; j++)  // DELETAR O ITEM E ATUALIZAR LISTA
            {  
                espera[j] = espera[j+1];  
            }  

            return pronto;
        }
    }
}
// =============== END ALGORITMO ROUND ROBIN =============== //

// =============== BEGIN ALGORITMO DE FILA =============== //
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
// =============== END ALGORITMO DE FILA =============== //