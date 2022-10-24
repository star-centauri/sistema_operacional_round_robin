#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

enum status { PARADO, BLOQUEADO, EXECUTANDO_CPU, EXECUTANDO_IO };
enum io { DISCO = 3, MAGNETICA = 4, IMPRESSORA = 7 };

#define QUANTUM 5
#define QTD_PROCESSOS 10
#define QTD_IO 5

/// @brief CUIDA DAS INFORMAÇÕES DE I/O DOS PROCESSADORES
/// @param tipo = qual o tipo de entrada e saida 
/// @param tempoDeEntrada = o tempo que o processo sai da execucao cpu e vai para fila de execucao io 
/// @param tempoIo = contador de quanto tempo ja teve de processo do programa no io
typedef struct
{
    enum io tipo;
    int tempoDeEntrada;
    int tempoIo;
} TipoIO;

/// @brief CONTEXTO DE INFORMAÇÃO DO PROCESSO
/// @param pid = identificador do processo
/// @param tempoChegada = tempo que o processo chegou para solicitar acesso a cpu
/// @param tempoDeServico = tempo necessario para o processo executar na cpu
/// @param situacao = situacao atual do processo
/// @param tempoProcessado = contador de quanto ja teve de processo do programa na cpu
/// @param entradaEhSaida = lista com a quantidade de acesso de entrada e saida do processo
typedef struct
{
    int pid;
    int tempoChegada;
    int tempoDeServico;
    enum status situacao;
    int tempoProcessado;
    TipoIO entradaEhSaida[QTD_IO];
} processo;

/// @brief ESTRUTURA DE UMA FILA
/// @param rows = array estatico com a quantidade de processos que serao executados
/// @param end = final da fila
typedef struct
{
    processo rows[QTD_PROCESSOS];
    int end;
} fifo;

/// @brief ESTRUTURA DE FILAS DE UM ESCALONADOR
/// @param altaPrioridade = fila que guarda os processos de alta prioridade de execucao na CPU
/// @param baixaPrioridade = fila que guarda os processos de baixa prioridade de execucao na CPU
/// @param entradaSaida = fila que guarda os processos que solicitaram entrada e saida
/// @param novo = guarda os processos novos a espera de entrar na CPU
typedef struct
{
    fifo altaPrioridade;
    fifo baixaPrioridade;
    fifo entradaSaida;
    processo novo[QTD_PROCESSOS];
} escalonador;

/// @brief TODAS AS FUNCOES DO PROJETO 
void enqueue(processo dado, fifo *f, char nome[20]);
void runProcesses(escalonador *filas);
void delay(int number_of_seconds);
void adicionarProcessoNovoFilaAlta(escalonador *filas, int elapsedTime);
void escreveArquivo(char string[], int pid);
int verificarSeExisteProcessoExecutar( escalonador *filas );
processo dequeue(fifo *f, char nome[20]);
processo getProcessosNovo(processo novo[QTD_PROCESSOS], int elapsedTime);
processo randomProcesso (int pid);
processo staticProcesso (int pid);
processo retornarProcessoExecutar(escalonador *filas);
processo executarProcessoIO(processo ioAtual, escalonador *filas);
fifo create();

int main() {
    // INICIALIZACAO DAS FILAS
   escalonador filas;
   filas.altaPrioridade = create();
   filas.baixaPrioridade = create();
   filas.entradaSaida = create();

    // CRIACAO DOS PROCESSOS DO PROGRAMA
   for (int i = 0; i < QTD_PROCESSOS; i++)
   {

        filas.novo[i] = randomProcesso(i+1);
        char str[100];

        printf("-----------------------------------------------\n");

        printf("PROCESSO CRIADO PID: %d \n", filas.novo[i].pid);
        sprintf(str, "PROCESSO CRIADO PID: %d \n", filas.novo[i].pid);
        escreveArquivo(str, filas.novo[i].pid);
        
        printf("TEMPO DE CHEGADA: %d \n", filas.novo[i].tempoChegada);
        sprintf(str, "TEMPO DE CHEGADA: %d \n", filas.novo[i].tempoChegada);
        escreveArquivo(str, filas.novo[i].pid);

        printf("TEMPO DE SERVICO: %d \n", filas.novo[i].tempoDeServico);
        sprintf(str, "TEMPO DE SERVICO: %d \n", filas.novo[i].tempoDeServico);
        escreveArquivo(str, filas.novo[i].pid);

        for (int j = 0; j < QTD_IO; j++)
        {
            if ( filas.novo[i].entradaEhSaida[j].tipo == DISCO )
            {
                printf("TIPO IO %s POR TEMPO DE CHEGADA %d \n", "DISCO", filas.novo[i].entradaEhSaida[j].tempoDeEntrada);
                sprintf(str, "TIPO IO %s POR TEMPO DE CHEGADA %d \n", "DISCO", filas.novo[i].entradaEhSaida[j].tempoDeEntrada);
                escreveArquivo(str, filas.novo[i].pid);
            }
            else if ( filas.novo[i].entradaEhSaida[j].tipo == MAGNETICA ) {
                printf("TIPO IO %s POR TEMPO DE CHEGADA %d \n", "MAGNETICA", filas.novo[i].entradaEhSaida[j].tempoDeEntrada);
                sprintf(str, "TIPO IO %s POR TEMPO DE CHEGADA %d \n", "MAGNETICA", filas.novo[i].entradaEhSaida[j].tempoDeEntrada);
                escreveArquivo(str, filas.novo[i].pid);
            }
            else if ( filas.novo[i].entradaEhSaida[j].tipo == IMPRESSORA ) {
                printf("TIPO IO %s POR TEMPO DE CHEGADA %d \n", "IMPRESSORA", filas.novo[i].entradaEhSaida[j].tempoDeEntrada);
                sprintf(str, "TIPO IO %s POR TEMPO DE CHEGADA %d \n", "IMPRESSORA", filas.novo[i].entradaEhSaida[j].tempoDeEntrada);
                escreveArquivo(str, filas.novo[i].pid);
            }
        }
        
        printf("-----------------------------------------------\n");

        delay(1);
   }
   
    runProcesses(&filas);

    return 0;
}

// =============== BEGIN ALGORITMO ROUND ROBIN =============== //

/// @brief A FUNCAO MESTRE, QUE EXECUTA TUDO
/// @param filas 
void runProcesses(escalonador *filas) {
    int run = 1; // MANTER EXECUÇÃO DO ALGORITMO ATÉ SE SOLICITADO PARAR
    int elapsedTime = 0; // SIMULA O TEMPO CORRIDO DENTRO DO LOOP (VAI SER IMPORTANTE PARA O TEMPO DE CHEGADA)
    char str[200];
    
    processo processoAtual;
    processoAtual.pid = 0; 
    
    processo ioAtual;
    ioAtual.pid = 0;
    
    // LOOP DE TODA EXECUÇÃO DO ALGORITMO
    while (run)
    {
        int countQuantum; // CONTA OS QUANTUM DO PROCESSO ATUAL NA CPU

        printf("UNIDADE DE TEMPO: %d \n", elapsedTime);    
        adicionarProcessoNovoFilaAlta(filas, elapsedTime);
        
        // EXECUTA IO              
        ioAtual = executarProcessoIO(ioAtual, filas);
        
        // BUSCAR PROCESSO PRONTO
        if ( processoAtual.pid == 0 ) {
            processoAtual = retornarProcessoExecutar(filas);
        }

        if ( processoAtual.pid != 0 ) {
            // LOOP QUANTUM
            for ( countQuantum = 0; countQuantum < QUANTUM; countQuantum++ )
            {
                printf("PROCESSO PRONTO PARA EXECUTAR PID: %d \n", processoAtual.pid);
                sprintf(str, "PROCESSO PRONTO PARA EXECUTAR PID %d TEMPO %d \n", processoAtual.pid, elapsedTime);
                escreveArquivo(str, processoAtual.pid);

                if ( (ioAtual.pid == 0 || ioAtual.pid != processoAtual.pid) && processoAtual.entradaEhSaida[0].tipo != 0 && processoAtual.entradaEhSaida[0].tempoDeEntrada <= elapsedTime ) {
                    sprintf(str, "PROCESSO PID %d ENTROU NA FILA DE I/O NO TEMPO %d.\n", processoAtual.pid, elapsedTime);
                    escreveArquivo(str, processoAtual.pid);
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
                    sprintf(str, "PROCESSO PID %d TERMINOU NO TEMPO %d.\n", processoAtual.pid, elapsedTime);
                    escreveArquivo(str, processoAtual.pid);
                    break;
                }

                processoAtual.tempoProcessado++;
                printf("PROCESSO PID %d EM EXECUCAO NA CPU, TEMPO QUE JA PROCESSOU DE %d E TEMPO RESTANTE %d \n", processoAtual.pid, processoAtual.tempoProcessado, (processoAtual.tempoDeServico - processoAtual.tempoProcessado));
                sprintf(str, "PROCESSO PID %d EM EXECUCAO NA CPU, TEMPO QUE JA PROCESSOU DE %d E TEMPO RESTANTE %d \n", processoAtual.pid, processoAtual.tempoProcessado, (processoAtual.tempoDeServico - processoAtual.tempoProcessado));
                escreveArquivo(str, processoAtual.pid);

                elapsedTime++;
                printf("UNIDADE DE TEMPO: %d \n", elapsedTime); 
            }
        }
        
        // PROCESSO NAO FINALIZOU NO PRIMEIRO QUANTUM, VAI PARA FILA DE BAIXA PRIORIDADE (PREEMPCAO)
        if ( processoAtual.pid != 0 &&  processoAtual.tempoProcessado != processoAtual.tempoDeServico ) {
            enqueue(processoAtual, &filas->baixaPrioridade, "baixaPrioridade");
        }

        // REGRA DE PARADA DO WHILE
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

/// @brief CUIDA DA EXECUCAO DO PROCESSO NO IO E O RETORNA A FILA DE BAIXA PRIORIDADE AO TERMINO
/// @param ioAtual 
/// @param filas 
/// @return processo
processo executarProcessoIO(processo ioAtual, escalonador *filas) {
    char str[200];

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
            sprintf(str, "PROCESSO PID %d EM EXECUCAO NO IO, TEMPO QUE JA PROCESSOU DE %d E TEMPO RESTANTE %d \n", ioAtual.pid, ioAtual.entradaEhSaida[0].tempoIo, (ioAtual.entradaEhSaida[0].tipo - ioAtual.entradaEhSaida[0].tempoIo));
            escreveArquivo(str, ioAtual.pid);
        }
    }

    return ioAtual;
}

/// @brief RETORNA O PROCESSO QUE ESTA NA FILA DE ALTA PRIORIDADE, SENAO, O DE BAIXA
/// @param filas 
/// @return processo
processo retornarProcessoExecutar(escalonador *filas) {
    processo executa = dequeue(&filas->altaPrioridade, "altaPrioridade");

    if ( executa.pid == 0 ) {
        executa = dequeue(&filas->baixaPrioridade, "baixaPrioridade");
    }         

    return executa;
}

/// @brief VERIFICA SE TEM PROCESSO NOVO NO TEMPO CORRIDO E O ADICIONA NA FILA DE ALTA PRIORIDADE
/// @param filas 
/// @param elapsedTime 
void adicionarProcessoNovoFilaAlta(escalonador *filas, int elapsedTime) {
    processo pronto = getProcessosNovo(filas->novo, elapsedTime);

    if ( pronto.pid != 0 ) {
        enqueue(pronto, &filas->altaPrioridade, "altaPrioridade");
    }
}

/// @brief VERIFICA DE TODAS AS FILAS ESTAO VAZIAS, SE NAO TEM NOVOS PROCESSOS E SE NAO TEM NENHUM PROCESSO EXECUTANDO NA CPU OU IO
/// @param filas 
/// @return flag
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

// =============== END ALGORITMO ROUND ROBIN =============== //

// =============== BEGIN ALGORITMO DE FILA =============== //

/// @brief INICIALIZA A CONTRUCAO DE UMA FILA
/// @return fifo
fifo create() {
    fifo fila;

    for (int i = 0; i < QTD_PROCESSOS; i++)
    {
        fila.rows[i].pid = 0;
    }
    
    fila.end = 0;

    return fila;
}

/// @brief INSERE DADOS NO FINAL DA FILA
/// @param dado 
/// @param f 
/// @param nome 
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

/// @brief REMOVE E RETORNA O PRIMEIRO DADO DA FILA
/// @param f 
/// @param nome 
/// @return processo
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

// =============== BEGIN AUXILIARES =============== //

/// @brief SIMULACAO DE UM TIMEOUT
/// @param number_of_seconds 
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

/// @brief CRIA DE FORMA RANDOMICA OS PROCESSOS 
/// @param pid 
/// @return processo
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

            int upper = proc.tempoChegada + proc.tempoDeServico;
            int lower = proc.tempoChegada;
            proc.entradaEhSaida[i].tempoDeEntrada = (rand() % (upper - lower + 1)) + lower;
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
    for (i = 0; i < (qtdIO-1); i++) {
        // Last i elements are already in place
        for (j = 0; j < (qtdIO - i - 1); j++) {
            if (proc.entradaEhSaida[j].tempoDeEntrada > proc.entradaEhSaida[j+1].tempoDeEntrada) {
                TipoIO temp = proc.entradaEhSaida[j];
                proc.entradaEhSaida[j] = proc.entradaEhSaida[j+1];
                proc.entradaEhSaida[j+1] = temp;
            }
        }
    }
 
    return proc;          
}

/// @brief UMA FUNCAO MESTRE PARA TENTAR A VERACIDADE DOS DADOS COM VALORES ESTATICOS DE PROCESSO
/// @param pid 
/// @return processo
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

/// @brief CRIA UM LOG DE CADA PROCESSO
/// @param string 
/// @param pid 
void escreveArquivo(char string[], int pid){
    
    char buff[10];    
    sprintf(buff, "%d", pid);
    char arquivo[20] = "processo";
    strcat(arquivo, buff);
    strcat(arquivo,".txt");

	FILE *pa;
	pa = fopen(arquivo, "a+");
	if (pa == NULL){
		printf("Arquivo nao pode ser aberto.");
	}

	fputs(string, pa);
	fclose(pa);	
}
// =============== END AUXILIARES =============== //
