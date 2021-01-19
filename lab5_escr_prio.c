#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#define N_ESCR 5
#define N_LEIT 7

int* vetor;
int vetor_tam = 500;
pthread_mutex_t mutex;
pthread_cond_t cond_l,cond_e;

short int leit = 0,escr = 0,e_fila = 0,inicio = 1;



void ini_leit(int id)
{
	pthread_mutex_lock(&mutex);
	while(escr > 0 || inicio ||e_fila > 0)//somente quando nao houver nenhum escritor esperando o leitor pode executar
	{
		printf("Leitor %d bloqueou-se, %d escritores na fila\n",id,e_fila);
		pthread_cond_wait(&cond_l,&mutex);
	}
	leit++;
	pthread_mutex_unlock(&mutex);
	printf("Leitor %d iniciando a leitura\n",id);
}

void term_leit(int id)
{
	pthread_mutex_lock(&mutex);
	leit--;
	if(leit == 0) pthread_cond_signal(&cond_e);
	printf("Leitor %d terminou, %d leitores na fila e %d escritores na fila\n",id,leit,e_fila);
	pthread_mutex_unlock(&mutex);
}

void* leitor(void* arg)
{
	int id = *(int*)arg;
	//verifica se pode ler
	ini_leit(id);
	//le
	float soma = 0;
	for(int i = 0; i < vetor_tam; i++)
	{
		printf("leitor %d %d\n", id, vetor[i]);
		soma += vetor[i];
	}
	printf("media do leitor %d:%f\n", id, soma/vetor_tam);
	//libera
	term_leit(id);
	pthread_exit(NULL);
}

void ini_escr(int id)
{
	pthread_mutex_lock(&mutex);
	inicio = 0;//inicializa o vetor
	e_fila++;//entra na fila
	while(escr >0|| leit >0)//espera outros escritores
	{
		printf("Escritor %d bloqueou-se, %d leitores na fila, %d escritores na fila\n",id, leit,e_fila);
		pthread_cond_wait(&cond_e,&mutex);
	}
	escr++;
	pthread_mutex_unlock(&mutex);	
	printf("Escritor %d iniciando a escrita\n",id);
}

void term_escr(int id)
{
	pthread_mutex_lock(&mutex);
	escr--;
	e_fila--;
	printf("Escritor %d terminou, %d leitores na fila e %d escritores na fila\n",id,leit,e_fila);
	pthread_cond_signal(&cond_e);
	pthread_cond_broadcast(&cond_l);
	pthread_mutex_unlock(&mutex);
}

void* escritor(void* arg)
{
	int id = *(int*)arg;
	//ve se pode escrever
	ini_escr(id);
	//escreve
	vetor[0] = id;
	for(int i = 1; i < vetor_tam-1; i++)
		vetor[i] = 2 * id;
	vetor[vetor_tam -1] = id;
	//termina a escrita
	term_escr(id);
	pthread_exit(NULL);
}



int main(int argc,char* argv[])
{
	printf("inicio\n");
	if(argc > 1) vetor_tam = atoi(argv[1]);
	if(vetor_tam <= 0)
	{
		printf("%d eh um tamanho invalido\n",vetor_tam);
		exit(-1);
	}
	vetor = malloc(sizeof(int) * vetor_tam);

	pthread_t ids_t[N_ESCR + N_LEIT];
	int* ids = malloc(sizeof(int) * (N_ESCR + N_LEIT));
	if(ids == NULL)
	{
		printf("falha ao alocar memoria\n");
		exit(-1);
	}
	
	pthread_mutex_init(&mutex,NULL);
	pthread_cond_init(&cond_l,NULL);
	pthread_cond_init(&cond_e,NULL);
	printf("criando as threads\n");
	//criar as threads
	for(int i = 0; i < N_ESCR + N_LEIT;i++)
	{
		if(i < N_ESCR)
		{
			ids[i] = i;
			if(pthread_create(&ids_t[i],NULL,(void*)escritor,(void*) &ids[i])) exit(-1);
		}
		else
		{
			ids[i] = i-N_ESCR;
			if(pthread_create(&ids_t[i],NULL,(void*)leitor,(void*) &ids[i])) exit(-1);
		}
		//printf("id%d\n",ids[i]);
	}
	for(int i = 0;i < N_ESCR + N_LEIT; i++)
		pthread_join(ids_t[i],NULL);
	printf("FIM\n");
	free(vetor);
	free(ids);
	pthread_exit(NULL);
	return 0;
}
