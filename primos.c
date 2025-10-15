#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <math.h>
#include <pthread.h>

#define FIM -1

long int *buffer;
int bufferSize;
sem_t mutex, slotCheio, slotVazio;
long int totalPrimos = 0;
long long int n;
long int out = 0;
int nthreads;

typedef struct
{
    int id;
    int totalPrimos;
} argR, vencedora;

int ehPrimo(long long int n)
{
    if (n <= 1)
        return 0;
    if (n == 2)
        return 1;
    if (n % 2 == 0)
        return 0;
    for (long long int i = 3; i <= sqrt(n); i += 2)
    {
        if (n % i == 0)
            return 0;
    }
    return 1;
}

void insereValor(long long int n)
{
    static int in = 0;
    sem_wait(&slotVazio);
    sem_wait(&mutex);
    buffer[in] = n;
    in = (in + 1) % bufferSize;
    sem_post(&mutex);
    sem_post(&slotCheio);
}

int retiraValor(long long int *n)
{
    sem_wait(&slotCheio);
    sem_wait(&mutex);
    *n = buffer[out];
    out = (out + 1) % bufferSize;
    sem_post(&mutex);
    sem_post(&slotVazio);
    return (*n != FIM);
}

void *produtor(void *arg)
{
    int id = *(int *)(arg);
    free(arg);

    for (long long int i = 1; i <= n; i++)
    {
        insereValor(i);
    }

    for (int i = 0; i < nthreads; i++)
    {
        insereValor(FIM);
    }

    printf("Produtor %d terminou\n", id);
    pthread_exit(NULL);
}

void *consumidor(void *arg)
{
    int id = *(int *)(arg);
    free(arg);

    int numPrimos = 0;
    long long int valor;

    argR *argReturn = malloc(sizeof(argR));
    if (argReturn == NULL)
    {
        perror("Erro ao alocar memória para retorno");
        pthread_exit(NULL);
    }

    while (1)
    {
        if (!retiraValor(&valor))
            break;

        if (ehPrimo(valor))
        {
            numPrimos++;
            printf("Consumidor %d encontrou primo: %lld\n", id, valor);
        }
    }

    argReturn->id = id;
    argReturn->totalPrimos = numPrimos;

    printf("Consumidor %d terminou com %d primos\n", id, numPrimos);

    pthread_exit(argReturn);
}

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        printf("Uso: %s <num_threads_consumidoras> <tamanho_buffer> <número_elementos>\n", argv[0]);
        exit(1);
    }

    nthreads = atoi(argv[1]);
    bufferSize = atoi(argv[2]);
    n = atoll(argv[3]);

    buffer = malloc(bufferSize * sizeof(long int));
    if (buffer == NULL)
    {
        perror("Erro ao alocar buffer");
        exit(1);
    }

    pthread_t *tid = malloc((nthreads + 1) * sizeof(pthread_t));
    if (tid == NULL)
    {
        perror("Erro ao alocar pthread_t");
        free(buffer);
        exit(1);
    }

    vencedora *vencedora = malloc(sizeof(vencedora));
    if (vencedora == NULL)
    {
        perror("Erro ao alocar vencedora");
        free(buffer);
        free(tid);
        exit(1);
    }
    vencedora->totalPrimos = 0;

    sem_init(&mutex, 0, 1);
    sem_init(&slotCheio, 0, 0);
    sem_init(&slotVazio, 0, bufferSize);

    int *argProd = malloc(sizeof(int));
    *argProd = 0;
    pthread_create(&tid[0], NULL, produtor, argProd);

    for (int i = 1; i <= nthreads; i++)
    {
        int *arg = malloc(sizeof(int));
        *arg = i;
        pthread_create(&tid[i], NULL, consumidor, arg);
    }

    for (int i = 1; i <= nthreads; i++)
    {
        argR *argReturn;
        pthread_join(tid[i], (void **)&argReturn);

        totalPrimos += argReturn->totalPrimos;
        if (argReturn->totalPrimos > vencedora->totalPrimos)
        {
            vencedora->totalPrimos = argReturn->totalPrimos;
            vencedora->id = argReturn->id;
        }
        free(argReturn);
    }

    pthread_join(tid[0], NULL);

    printf("Total de primos: %ld\n", totalPrimos);
    printf("Vencedora: Consumidor %d com %d primos\n", vencedora->id, vencedora->totalPrimos);

    free(vencedora);
    free(buffer);
    free(tid);

    sem_destroy(&mutex);
    sem_destroy(&slotCheio);
    sem_destroy(&slotVazio);

    return 0;
}
