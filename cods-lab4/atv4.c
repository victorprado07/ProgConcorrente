#include <stdio.h>
#include <stdlib.h> 
#include <pthread.h>
#include <math.h>
#include "timer.h"

long int soma = 0; //variavel compartilhada entre as threads
pthread_mutex_t mutex; //variavel de lock para exclusao mutua
double inicio, fim, delta;

typedef struct {
  int n; 
  int t;  
  int div;
} ThreadArgs;

//funcao que verifica se 1 numero e primo
int ehPrimo(long long int n) {
  int i;
  if(n<=1) return 0;
  if(n==2) return 1;
  if(n%2==0) return 0;
  for(i=3; i<sqrt(n)+1; i+=2)
    if(n%i==0) return 0;
  return 1;
}

void *executa (void *arg) {
  ThreadArgs* args = (ThreadArgs*)arg;
  int id = (int) (pthread_self() % args->t); 

  // Calcula o intervalo de números para essa thread
  int extra = args->n%args->t;
  int start = id * args->div + (id < extra ? id : extra);
  int end = start + args->div - 1 + (id < extra ? 1 : 0);


  for (long long int i=start; i<end; i++) {
    if (ehPrimo(i)){
      pthread_mutex_lock(&mutex);
      soma++;
      printf("id= %d  soma= %ld \n", id, soma); 
      pthread_mutex_unlock(&mutex);
    }
  }
  pthread_exit(NULL);
}

//fluxo principal
int main(int argc, char *argv[]) {
  pthread_t *tid; //identificadores das threads no sistema
  int nthreads, divisao; //qtde de threads (passada linha de comando)
  long long int n;
  //--le e avalia os parametros de entrada
  if(argc<3) {
    printf("Digite: %s <numero de threads> <N>\n", argv[0]);
    return 1;
  }
  nthreads = atoi(argv[1]);
  n = atoi(argv[2]);
  divisao = n/nthreads;

  ThreadArgs args = {n, nthreads, divisao}; 

  GET_TIME(inicio);

  //--aloca as estruturas
  tid = (pthread_t*) malloc(sizeof(pthread_t)*(nthreads+1));
  if(tid==NULL) {puts("ERRO--malloc"); return 2;}

  pthread_mutex_init(&mutex, NULL);//--inicilaiza o mutex (lock de exclusao mutua)
 
  for(long int t=0; t<nthreads; t++) { //--cria as threads
    if (pthread_create(&tid[t], NULL, executa, (void *)&args)) {
      printf("--ERRO: pthread_create()\n"); exit(-1);
    }
  }

  //--espera todas as threads terminarem
  for (int t=0; t<nthreads; t++) {
    if (pthread_join(tid[t], NULL)) {
        printf("--ERRO: pthread_join() \n"); exit(-1); 
    } 
  } 
  GET_TIME(fim);
  delta = fim - inicio;
  //--finaliza o mutex
  pthread_mutex_destroy(&mutex);
  
  printf("Valor de 'soma' = %ld\n foi calculado em %lf seg", soma, delta);

  return 0;
}
