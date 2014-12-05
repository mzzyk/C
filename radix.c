//this ia an example of radix sort by pthread on linux
// compile command :  g++ -o radix -pthread radix.c    # -pthread is imptant to program of pthread on linux
// run command : ./radix 4 DATA1   #  4 is the number of pthread; DATA1 is a file of some integers to test
#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<semaphore.h>

#define MAX 32
#define MAX_SIZE 100000
#define MAX_BUCKET 10

int counter=0;
int n;//the number of data
int data[MAX_SIZE];
int temp[MAX_SIZE];
int bucket[MAX_BUCKET];
int k;//the number of threads
int max_num;
int pow_10=1;

pthread_mutex_t mutex;
pthread_cond_t cond_var;
pthread_t tid[MAX_SIZE];
sem_t sem_i;
sem_t sem_bucket;

void get_max_num();
void scan();
//void get_sum(int pow_10);
void *radix_sort(void *rank);

int main(int *argc,char *argv[]){
	FILE *fp;
	int i=0;
	if((fp=fopen(argv[2],"r"))==NULL){
		printf("can't open the file!");
	exit(0);
	}
	while(fscanf(fp,"%d",&data[i])!=EOF)
	i++;
	n = i;
	
	k = (int)atoi(argv[1]);
	
	if((k <= 0)||(k >= MAX))
	{
		printf("invalid thread count -- defaulting to 4\n");
		k = 4;
	}
	
	get_max_num();
	
	sem_init(&sem_i,0,1);
	sem_init(&sem_bucket,0,1);

	for(i = 0; i < k; i++){    //create pthreads
		sem_wait(&sem_i);
		int tmp = i;
		pthread_create(&tid[i],NULL,radix_sort,&tmp);
	}

	for(i = 0; i < k; i++)   //destroy pthreads
		pthread_join(tid[i],NULL);

	printf("the smallest number is: %d\n",data[0]);
	printf("the n/4 smallest number is: %d\n",data[n/4 - 1]);
	printf("the n/2 smallest number is: %d\n",data[n/2 - 1]);
	printf("the largest number is: %d\n",data[n - 1]);
	
	return 0;
}

//look for the max number
void get_max_num(){
	int i;
	max_num = data[0];
	for(i = 0; i < n; i++)
		if(data[i] > max_num)
		max_num = data[i];

}

void scan(){
	int i;
	for(i = 1; i < MAX_BUCKET; i++)
		bucket[i] += bucket[i-1];
	return;
}

void *radix_sort(void *rank){  //radix_sort
	int my_rank = *((int *)rank);
	sem_post(&sem_i);
	
	int i,j;
	int my_first = my_rank * (n / k);
	int my_last;
	int size;
	
	if(my_rank == k-1)
		my_last = n-1;
	else
		my_last = my_first + n/k -1;
	size = my_last - my_first + 1;

	while((max_num / pow_10) > 0){
		for(i = my_first; i <= my_last; i++){
		sem_wait(&sem_bucket);
		bucket[(data[i] / pow_10) % MAX_BUCKET]++;   //get the sum 
		sem_post(&sem_bucket);
		}

	pthread_mutex_lock(&mutex);
	counter++;
	if(counter == k){
		counter = 0;
		scan();

		for(i = (n - 1) ;i >= 0; i--) 
			temp[--bucket[(data[i] / pow_10) % MAX_BUCKET]] = data[i];

		for(i = 0; i < n; i++) //copy back
			data[i] = temp[i];
		
		for(i = 0; i < MAX_BUCKET; i++)
			bucket[i] = 0; 
		pow_10 *= 10;

		pthread_cond_broadcast(&cond_var);
	}
	else{
		while(pthread_cond_wait(&cond_var,&mutex) != 0);
	}
	pthread_mutex_unlock(&mutex);

	}

}
