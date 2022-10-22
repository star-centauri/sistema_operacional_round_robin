#include <stdio.h>
#include <stdlib.h>
#include <time.h>

enum status { PARADO, BLOQUEADO, EXECUTANDO_CPU, EXECUTANDO_IO };
enum io { DISCO = 3, MAGNETICA = 4, IMPRESSORA = 7 };

#define QUANTUM 5
#define QTD_PROCESSOS 1
#define QTD_IO 5

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
    TipoIO entradaEhSaida[QTD_IO];
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
. lista de novo: todos os processos
*/
typedef struct
{
    fifo altaPrioridade;
    fifo baixaPrioridade;
    fifo entradaSaida;
    processo novo[QTD_PROCESSOS];
} escalonador;

fifo create();
void enqueue(processo dado, fifo *f, char nome[20]);
processo dequeue(fifo *f, char nome[20]);
void runProcesses(escalonador *filas);
processo getProcessosNovo(processo novo[QTD_PROCESSOS], int elapsedTime);
processo randomProcesso (int pid);
int verificarSeExisteProcessoExecutar( escalonador *filas );
void delay(int number_of_seconds);
processo staticProcesso (int pid);
void adicionarProcessoNovoFilaAlta(escalonador *filas, int elapsedTime);
processo retornarProcessoExecutar(escalonador *filas);
processo executarProcessoIO(processo ioAtual, escalonador *filas);

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
        //filas.novo[i] = staticProcesso(i+1);
        filas.novo[i] = randomProcesso(i+1);

        printf("-----------------------------------------------\n");

        printf("PROCESSO CRIADO PID: %d \n", filas.novo[i].pid);
        printf("TEMPO DE CHEGADA: %d \n", filas.novo[i].tempoChegada);
        printf("TEMPO DE SERVICO: %d \n", filas.novo[i].tempoDeServico);

        for (int j = 0; j < QTD_IO; j++)
        {
            if ( filas.novo[i].entradaEhSaida[j].tipo == DISCO )
            {
                printf("TIPO IO %s POR TEMPO DE CHEGADA %d \n", "DISCO", filas.novo[i].entradaEhSaida[j].tempoDeEntrada);
            }
            else if ( filas.novo[i].entradaEhSaida[j].tipo == MAGNETICA ) {
                printf("TIPO IO %s POR TEMPO DE CHEGADA %d \n", "MAGNETICA", filas.novo[i].entradaEhSaida[j].tempoDeEntrada);
            }
            else if ( filas.novo[i].entradaEhSaida[j].tipo == IMPRESSORA ) {
                printf("TIPO IO %s POR TEMPO DE CHEGADA %d \n", "IMPRESSORA", filas.novo[i].entradaEhSaida[j].tempoDeEntrada);
            }
        }
        
        printf("-----------------------------------------------\n");

        delay(1);
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

        printf("UNIDADE DE TEMPO: %d \n", elapsedTime);    
        adicionarProcessoNovoFilaAlta(filas, elapsedTime);
        
        // EXECUTA IO              
        ioAtual = executarProcessoIO(ioAtual, filas);
        
        if ( processoAtual.pid == 0 ) {
            processoAtual = retornarProcessoExecutar(filas);
        }

        if ( processoAtual.pid != 0 ) {
            // LOOP QUANTUM
            for ( countQuantum = 0; countQuantum < QUANTUM; countQuantum++ )
            {
                printf("PROCESSO PRONTO PID: %d \n", processoAtual.pid);

                if ( ioAtual.pid == 0 && processoAtual.entradaEhSaida[0].tipo != 0 && processoAtual.entradaEhSaida[0].tempoDeEntrada <= processoAtual.tempoProcessado ) {
                    enqueue(processoAtual, &filas->entradaSaida, "entradaSaida");
                    processoAtual.pid = 0;
                }

                // SAIR DO QUANTUM QUANDO PROCESSO FOR IO
                if (processoAtual.pid == 0) {
                    break;
                }

                // PROCESSO TERMINOU ANTES DO QUANTUM 
                if ( processoAtual.tempoProcessado == processoAtual.tempoDeServico ) { 
                    printf("PROCESSO PID %d TERMINOU.\n", processoAtual.pid);
                    break;
                }

                processoAtual.tempoProcessado++;
                printf("PROCESSO PID %d EM EXECUCAO, TEMPO QUE JA PROCESSOU DE %d E TEMPO RESTANTE %d \n", processoAtual.pid, processoAtual.tempoProcessado, (processoAtual.tempoDeServico - processoAtual.tempoProcessado));
                elapsedTime++;
            }
        }
        
        // PROCESSO NAO FINALIZOU NO PRIMEIRO QUANTUM, VAI PARA ESPERA
        if ( processoAtual.pid != 0 &&  processoAtual.tempoProcessado != processoAtual.tempoDeServico ) {
            enqueue(processoAtual, &filas->baixaPrioridade, "baixaPrioridade");
        }

        int existProcesso = verificarSeExisteProcessoExecutar(filas);
        if ( !existProcesso && ioAtual.pid == 0 && processoAtual.pid == 0 ) {
            run = 0;
        }
        else {
            processoAtual.pid = 0;
            elapsedTime++;
        }
    }
    
}

processo executarProcessoIO(processo ioAtual, escalonador *filas) {
    if ( ioAtual.pid == 0 ) {
        processo executaIo = dequeue(&filas->entradaSaida, "entradaSaida");
        
        if( executaIo.pid != 0 ) {
            ioAtual = executaIo;
        }
    }
    
    if ( ioAtual.pid != 0 ) {
        if( ioAtual.entradaEhSaida[0].tempoIo != 0 && ioAtual.entradaEhSaida[0].tempoIo == ioAtual.entradaEhSaida[0].tipo ) {
            if ( ioAtual.entradaEhSaida[0].tipo == DISCO ) {
                // DELETAR IO E ATUALIZAR LISTA
                int id;
                for (id = 0; id < (QTD_IO-1); id++)
                {
                    ioAtual.entradaEhSaida[id] = ioAtual.entradaEhSaida[id+1];
                }
                ioAtual.entradaEhSaida[id].tipo = 0;

                enqueue(ioAtual, &filas->baixaPrioridade, "baixaPrioridade");
            }
            else {
                // DELETAR IO E ATUALIZAR LISTA
                int id;
                for (id = 0; id < (QTD_IO-1); id++)
                {
                    ioAtual.entradaEhSaida[id] = ioAtual.entradaEhSaida[id+1];
                }
                ioAtual.entradaEhSaida[id].tipo = 0;

                enqueue(ioAtual, &filas->altaPrioridade, "altaPrioridade");
            }
            
            ioAtual.pid = 0;
        }
        else {
            ioAtual.entradaEhSaida[0].tempoIo++;
            printf("PROCESSO PID %d EM EXECUCAO NO IO, TEMPO QUE JA PROCESSOU DE %d E TEMPO RESTANTE %d \n", ioAtual.pid, ioAtual.entradaEhSaida[0].tempoIo, (ioAtual.entradaEhSaida[0].tipo - ioAtual.entradaEhSaida[0].tempoIo));
        }
    }

    return ioAtual;
}

processo retornarProcessoExecutar(escalonador *filas) {
    processo executa = dequeue(&filas->altaPrioridade, "altaPrioridade");

    if ( executa.pid == 0 ) {
        executa = dequeue(&filas->baixaPrioridade, "baixaPrioridade");
    }         

    return executa;
}

void adicionarProcessoNovoFilaAlta(escalonador *filas, int elapsedTime) {
    processo pronto = getProcessosNovo(filas->novo, elapsedTime);

    if ( pronto.pid != 0 ) {
        enqueue(pronto, &filas->altaPrioridade, "altaPrioridade");
    }
}

int verificarSeExisteProcessoExecutar( escalonador *filas ) {
    if ( filas->altaPrioridade.end != 0 )
    {
        return 1;
    }

    if ( filas->baixaPrioridade.end != 0 )
    {
        return 1;
    }
    
    if ( filas->entradaSaida.end != 0 ) {
        return 1;
    }

    for (int i = 0; i < QTD_PROCESSOS; i++)
    {
        if ( filas->novo[i].pid != 0 )
        {
            return 1;
        }
    }
    
    return 0;
}

/// @brief FAZ A BUSCAR PELOS PROCESSOS NOVOS QUE ESTÃO PRONTO PARA O ESCALONADOR
/// @param novo 
/// @param elapsedTime 
/// @return processo 
processo getProcessosNovo(processo novo[QTD_PROCESSOS], int elapsedTime) {
    processo pronto;
    pronto.pid = 0;

    for (int i = 0; i < QTD_PROCESSOS; i++)
   {
        if ( novo[i].pid != 0 &&  novo[i].tempoChegada <= elapsedTime )
        {
            pronto = novo[i];
            novo[i].pid = 0;
        }    
   }
    return pronto;
}

processo randomProcesso (int pid) {
    int segundos = time(0);
    processo proc;
    srand(segundos);
    int qtdIO = rand() % QTD_IO;

    proc.pid = pid;
    proc.tempoChegada = pid*2;
    proc.tempoDeServico = (rand() % 10)+1;
    proc.situacao = PARADO;
    proc.tempoProcessado = 0;
    
    for (int i = 0; i < QTD_IO; i++)
    {
        if ( i <= qtdIO ) {
            int qualTipo = (rand() % 3) + 1;
            
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

            proc.entradaEhSaida[i].tempoDeEntrada = (rand() % (proc.tempoDeServico-1)) + 1;
            proc.entradaEhSaida[i].tempoIo = 0;
            delay(1);
        }
        else {
            proc.entradaEhSaida[i].tipo = 0;
            proc.entradaEhSaida[i].tempoDeEntrada = 0;
            proc.entradaEhSaida[i].tempoIo = 0;
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

processo staticProcesso (int pid) {
    processo proc;
    
    proc.pid = pid;
    proc.tempoChegada = 2;
    proc.tempoDeServico = 9;
    proc.situacao = PARADO;
    proc.tempoProcessado = 0;
    
    proc.entradaEhSaida[0].tipo = MAGNETICA;
    proc.entradaEhSaida[0].tempoDeEntrada = 6;
    proc.entradaEhSaida[0].tempoIo = 0;

    proc.entradaEhSaida[1].tipo = 0;
    proc.entradaEhSaida[1].tempoDeEntrada = 0;
    proc.entradaEhSaida[1].tempoIo = 0;

    proc.entradaEhSaida[2].tipo = 0;
    proc.entradaEhSaida[2].tempoDeEntrada = 0;
    proc.entradaEhSaida[2].tempoIo = 0;

    proc.entradaEhSaida[3].tipo = 0;
    proc.entradaEhSaida[3].tempoDeEntrada = 0;
    proc.entradaEhSaida[3].tempoIo = 0;

    proc.entradaEhSaida[4].tipo = 0;
    proc.entradaEhSaida[4].tempoDeEntrada = 0;
    proc.entradaEhSaida[4].tempoIo = 0;
    
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
        printf("FILA %s ESTA CHEIA!\n", nome);
    }
    else {
        f->rows[f->end] = dado;
        f->end++;
        printf("PROCESSO DE PID %d ADICIONADO A FILA %s \n", dado.pid, nome);
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
        printf("PROCESSO DE PID %d REMOVIDO DA FILA %s \n", backup.pid, nome);
        return backup;
    }
}
// =============== END ALGORITMO DE FILA =============== //

void delay(int number_of_seconds)
{
    // Converting time into milli_seconds
    int milli_seconds = 1000 * number_of_seconds;
 
    // Storing start time
    clock_t start_time = clock();
 
    // looping till required time is not achieved
    while (clock() < start_time + milli_seconds)
        ;
}