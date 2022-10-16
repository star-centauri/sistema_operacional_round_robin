#include <stdio.h>
#include <stdlib.h>
#include <time.h>

enum status { PARADO, BLOQUEADO, EXECUTANDO_CPU, EXECUTANDO_IO };
enum io { DISCO = 3, MAGNETICA = 5, IMPRESSORA = 7 };

#define QUANTUM 5
#define QTD_PROCESSOS 10

/*
Criar um contrutor chamado TipoIO que conterá:
. enum io Tipo = guarda o tipo de IO e junto seu tempo de execução "DISCO", "MAGNETICA", "IMPRESSORA"
. int tempoDeEntrada = o momento que o processo irá entrar na fila de IO
*/
typedef struct
{
    enum io tipo;
    int tempoDeEntrada;
    int tempoIo;
} TipoIO;

/*
Criar um contrutor chamado PROCESSO que conterá:
. int PID = identificador do processo
. int Prioridade = quanto menor maior a prioridade
. int tempoChegada = tempo que o processo está pronto para execução
. int tempoDeServico = tempo de execução total do processo
. enum status situacao = entender em qual situação o processo se encontra em um determinado instante
. int tempoProcessado = tempo que o processo já executou
. enum TipoIO entradaEhSaida[5] = as entradas ao IO, quais e em que tempo (caso tenha) 
*/
typedef struct
{
    int pid;
    int tempoChegada;
    int tempoDeServico;
    enum status situacao;
    int tempoProcessado;
    TipoIO entradaEhSaida[5];
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
. espera de novo: todos os processos
. lista de novo: todos os processos
*/
typedef struct
{
    fifo altaPrioridade;
    fifo baixaPrioridade;
    fifo entradaSaida;
    processo espera[QTD_PROCESSOS];
    processo novo[QTD_PROCESSOS];
} escalonador;

fifo create();
void enqueue(processo dado, fifo *f, char nome[20]);
processo dequeue(fifo *f, char nome[20]);
void printProcessos(fifo *f);
void runProcesses(escalonador *filas);
processo getProcessosNovo(processo novo[QTD_PROCESSOS], int elapsedTime);
processo randomProcesso (int pid);

int main() {
    /*
    Menu:
     1 - Introdução
     2 - Executar os processos
     3 - Parar processos
     4 - Detalhes
    */
   escalonador filas;
   filas.altaPrioridade = create();
   filas.baixaPrioridade = create();
   filas.entradaSaida = create();

   for (int i = 0; i < QTD_PROCESSOS; i++)
   {
        filas.novo[i] = randomProcesso(i+1);
        printf("PROCESSO CRIADO PID: %d\n", filas.novo[i].pid);
   }
   
    runProcesses(&filas);

    return 0;
}

// =============== BEGIN ALGORITMO ROUND ROBIN =============== //
void runProcesses(escalonador *filas) {
    int run = 1; // MANTER EXECUÇÃO DO ALGORITMO ATÉ SE SOLICITADO PARAR
    int elapsedTime = 0; // SIMULA O TEMPO CORRIDO DENTRO DO LOOP (VAI SER IMPORTANTE PARA O TEMPO DE CHEGADA)
    processo processoAtual;
    processoAtual.pid = 0; 
    processo ioAtual;
    ioAtual.pid = 0;
    
    // LOOP DE TODA EXECUÇÃO DO ALGORITMO
    while (run)
    {
        int countQuantum;
        
        processo executa;
        if ( processoAtual.pid == 0 ) {
            executa = dequeue(&filas->altaPrioridade, "altaPrioridade");

            if ( executa.pid != 0 ) {
                processoAtual = executa;
                printf("PROCESSO ATUAL PID: %d\n", processoAtual.pid);
            }
            else {
                executa = dequeue(&filas->baixaPrioridade, "baixaPrioridade");

                if ( executa.pid != 0 ) {
                    processoAtual = executa;
                    printf("PROCESSO ATUAL PID: %d\n", processoAtual.pid);
                }
            }
        }

        // LOOP CPU
        for ( countQuantum = 0; countQuantum < QUANTUM; countQuantum++ )
        {
            // VERIFIFCO SE TEM ALGUM PROCESSO NOVO PARA ADICIONAR A LISTA DE PRIORIDADES
            processo pronto = getProcessosNovo(filas->novo, elapsedTime);
            printf("PROCESSO PRONTO: %d\n", pronto.pid);

            if ( pronto.pid != 0 ) {
                enqueue(pronto, &filas->altaPrioridade, "altaPrioridade");
            }
            
            // SE TEM PROCESSO RODANDO
            if ( processoAtual.pid == 0 ) {
                break;
            }

            //SE VAI PARA IO
            if ( processoAtual.entradaEhSaida[0].tempoDeEntrada <= processoAtual.tempoProcessado ) {
                printf("PROCESSO PID %d VAI ENTRAR PARA IO %d\n", processoAtual.pid, processoAtual.entradaEhSaida[0].tipo);
                enqueue(processoAtual, &filas->entradaSaida, "entradaSaida");

                if ( ioAtual.pid == 0 ) {
                    processo executaIo = dequeue(&filas->entradaSaida, "entradaSaida");
                    
                    if( executaIo.pid != 0 ) {
                        ioAtual = executaIo;
                    }
                }

                if ( ioAtual.pid != 0 ) {
                    if( ioAtual.entradaEhSaida[0].tempoIo == ioAtual.entradaEhSaida[0].tipo ) {
                        
                        if ( ioAtual.entradaEhSaida[0].tipo == DISCO ) {
                            enqueue(ioAtual, &filas->baixaPrioridade, "baixaPrioridade");
                        }
                        else {
                            enqueue(ioAtual, &filas->altaPrioridade, "altaPrioridade");
                        }
                        
                        ioAtual.pid = 0;
                    }

                    ioAtual.entradaEhSaida[0].tempoIo++;
                }
            }

            // SE PROCESSO FOI PARA IO
            if ( processoAtual.pid == 0 ) {
                break;
            }

            // PARAR LOOP CASO JA TENHA FEITO O PROCESSO POR COMPLETO
            if ( processoAtual.tempoProcessado == processoAtual.tempoDeServico ) { 
                printf("PROCESSO PID %d TERMINOU.\n", processoAtual.pid);
                break;
            }

            processoAtual.tempoProcessado++;
            printf("PROCESSO PID %d EM EXECUÇÃO, TEMPO QUE JÁ PROCESSOU DE %d E TEMPO RESTANTE\n", processoAtual.pid, processoAtual.tempoProcessado, (processoAtual.tempoDeServico - processoAtual.tempoProcessado));
        }

        // PROCESSO NAO FINALIZOU NO PRIMEIRO QUANTUM, VAI PARA ESPERA
        if ( processoAtual.pid != 0 &&  processoAtual.tempoProcessado != processoAtual.tempoDeServico ) {
            enqueue(processoAtual, &filas->baixaPrioridade, "baixaPrioridade");
        }

        processoAtual.pid = 0;
        elapsedTime++;
        //Checar se todas as filas estão vazias e terminar run = 0;
    }
    
}

/// @brief FAZ A BUSCAR PELOS PROCESSOS NOVOS QUE ESTÃO PRONTO PARA O ESCALONADOR
/// @param espera 
/// @param elapsedTime 
/// @return processo 
processo getProcessosNovo(processo novo[QTD_PROCESSOS], int elapsedTime) {
    processo pronto;
    pronto.pid = 0;
    
    for (int i = 0; i < QTD_PROCESSOS; i++)
    {
        printf("TEMPO CHEGADA: %d\n", novo[i].tempoChegada);
        printf("PID %d\n", pronto.pid);
        if ( novo[i].pid != 0 && novo[i].tempoChegada <= elapsedTime) {
            printf("PID %d\n", pronto.pid);
            pronto = novo[i];

            novo[i].pid = 0;
        }
    }
    // printf("PID %d\n", pronto.pid);
    // printf("ELAPSED %d\n", elapsedTime);
    return pronto;
}

processo randomProcesso (int pid) {
    processo proc;
    srand(time(NULL));
    int qtdIO = rand() % 6;

    proc.pid = pid;
    proc.tempoChegada = pid*2;
    proc.tempoDeServico = (rand() % 10)+1;
    proc.situacao = PARADO;
    proc.tempoProcessado = 0;
    
    for (int i = 0; i < qtdIO; i++)
    {
        int qualTipo = (rand() % 3) + 1;
        proc.entradaEhSaida[i].tempoDeEntrada = (rand() % (proc.tempoDeServico-1))+1;
        
        if ( qualTipo == 1 ) {
            proc.entradaEhSaida[i].tipo = DISCO;
        }
        else if ( qualTipo == 2 )
        {
            proc.entradaEhSaida[i].tipo = MAGNETICA;
        }
        else {
            proc.entradaEhSaida[i].tipo = IMPRESSORA;
        }  
    }
    
    // SORT
    int i, j;
    for (i = 0; i < 4; i++) {
        // Last i elements are already in place
        for (j = 0; j < 5 - i - 1; j++) {
            if (proc.entradaEhSaida[j].tempoDeEntrada > proc.entradaEhSaida[j+1].tempoDeEntrada) {
                TipoIO temp = proc.entradaEhSaida[j];
                proc.entradaEhSaida[j] = proc.entradaEhSaida[j+1];
                proc.entradaEhSaida[j+1] = temp;
            }
        }
    }
 
    return proc;          
}
// =============== END ALGORITMO ROUND ROBIN =============== //

// =============== BEGIN ALGORITMO DE FILA =============== //
fifo create() {
    fifo fila;

    for (int i = 0; i < QTD_PROCESSOS; i++)
    {
        fila.rows[i].pid = 0;
    }
    
    fila.end = 0;

    return fila;
}

void enqueue(processo dado, fifo *f, char nome[20]) {
    if ( f->end == QTD_PROCESSOS ) {
        printf("FILA %s ESTÁ CHEIA!\n", nome);
    }
    else {
        f->rows[f->end] = dado;
        f->end++;
        printf("PROCESSO DE PID %d ADICIONADO A FILA %S\n", dado.pid, nome);
    }
}

processo dequeue(fifo *f, char nome[20]) {
    processo backup;

    if ( f->end == 0 ) {
        printf("FILA %s VAZIA!\n", nome);
        backup.pid = 0;
        return backup;
    }
    else {
        backup = f->rows[0];

        for (int i = 0; i < f->end; i++)
        {
            f->rows[i] = f->rows[i+1];
        }
        
        f->end--;
        printf("PROCESSO DE PID %d REMOVIDO DA FILA %s\n", backup.pid, nome);
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