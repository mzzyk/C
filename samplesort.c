//this is an example of sample_sort by pthread on linux
//run command : ./samplesort 4 DATA1   #4 is the number of pthread; DATA1 is file of data to test
#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<semaphore.h>
#include<unistd.h>

#define  MAX 32
#define MAX_SIZE 100000

int n;   //the number of data
int no_threads;// the number of threads
int data[MAX_SIZE];//DATA
int count;
int thread;
int number;

sem_t sem_sample;
sem_t sem_spl;
sem_t sem_i;
sem_t sem_num;
pthread_t tid[MAX];

void *sort(void *rank);
int *sample;
int *splitter;
int *bucket_size;

int comp(const void *a,const void *b);

int comp(const void *a,const void *b)
{
	return *((int *)a) - *((int *)b);
}
int main(int * argc, char * argv[])
{
	int i;
	char *c;
        
	FILE *fp;
	if((fp = fopen(argv[2],"r")) == NULL)
	{ 
		printf("can't open this file!");
		exit(0);
	}
	
	i = 0;
	while(fscanf(fp,"%d",&data[i]) != EOF) i++;
	n = i;

	no_threads = (int)atoi(argv[1]);
	if((no_threads <= 0)||(no_threads >= MAX))
	{
		printf("invalid thread count -- defaulting to 4\n");
		no_threads = 4;
	}
	
	sem_init(&sem_sample,0,0);    //initialize the mutual exclusion semaphor
	sem_init(&sem_spl,0,0);
	sem_init(&sem_i,0,1);
	sem_init(&sem_num,0,1);

	
	splitter = (int *)malloc((no_threads - 1) * sizeof(int));
	sample = (int *)malloc((no_threads - 1) * no_threads * sizeof(int));
	bucket_size = (int *)malloc(no_threads*sizeof(int));
	
	for(i = 0; i < no_threads; i++){
	sem_wait(&sem_i);	
	int temp = i;
	pthread_create(&tid[i],NULL,sort,&temp); //create pthreads.
	}
	
	for(i = 0;i < no_threads;i++)    //destroy pthread
		pthread_join(tid[i],NULL);
	for(i = 0;i < no_threads;i++) //print the elems of every bucket
	{
	//	printf("the size of %dth bucket is : %d\n",i,bucket_size[i]);
	}
	
	printf("the smallest number is %d\n",data[0]);
	printf("the 1/4 samllest number is %d\n",data[n/4-1]);
	printf("the 1/2 samllest number is %d\n",data[n/2-1]);
	printf("the largest number is %d\n",data[n-1]);
	return 0;
}

void *sort(void *rank)  //sample sort
{
	int my_rank = *((int*)rank);
	sem_post(&sem_i);
	int my_n = n/no_threads;
	int my_first_i;
	int my_last_i;
	int space,size;
	int i,j;
	int *my_data = (int *)malloc( (2 * n/no_threads) * sizeof(int));
	
	my_first_i = my_rank * (n/no_threads);
	
	if(my_rank == no_threads-1)
		my_last_i = n - 1;
	else
		my_last_i = my_first_i + my_n - 1;

	size=my_last_i - my_first_i + 1;
	space = (my_last_i - my_first_i + 1)/no_threads;	

	for(i = 0,j = my_first_i;i < my_n,j < my_last_i;i++,j++) //evenly assign the numbers to every bucket
		my_data[i] = data[j];
	
	qsort(my_data, size, sizeof(int), comp); //sort in bucket

	for(i = 1; i < size; i++) //select samples
		if(i%space == 0)
			sample[my_rank * (no_threads - 1) + i/space - 1] = my_data[i];
	sem_post(&sem_sample);


	if(my_rank == no_threads-1){
		for(i = 0;i < no_threads;i++)
			sem_wait(&sem_sample);
		qsort(sample,no_threads*(no_threads-1),sizeof(int),comp); //sort samples
		space = no_threads-1;
		for( i = 0; i < no_threads-1; i++) //select splitters
			splitter[i] = sample[(i+1)*space];

		for(i = 0;i < no_threads - 1;i++){  //print splitters
	//		printf("splitter[%d]=%d\n",i,splitter[i]);
		}	

		for(i = 0; i < no_threads; i++)
			sem_post(&sem_spl);
	}

	sem_wait(&sem_spl);
	
	if(my_rank == 0){ //assign numbers by splitters
		for(i = 0, j = 0; i < n; i++)
			if(data[i] <= splitter[0])
				my_data[j++] = data[i];
		my_n = j;
		bucket_size[my_rank] = my_n;
		sem_wait(&sem_num);
		number++;
		sem_post(&sem_num);
	}
	else if(my_rank == (no_threads-1))
	{
		for(i = 0,j = 0; i < n; i++)
		if(data[i] > splitter[no_threads-2])
			my_data[j++] = data[i];
		my_n = j;
		bucket_size[my_rank] = my_n;
		sem_wait(&sem_num);
		number++;
		sem_post(&sem_num);
	}
	
	else{
		for(i = 0,j = 0;i < n;i++)
		if(data[i] <= splitter[my_rank] && data[i] > splitter[my_rank-1])
			my_data[j++] = data[i];
		my_n = j;
		bucket_size[my_rank] = my_n;
		sem_wait(&sem_num);
		number++;
		sem_post(&sem_num);
	}
	while(number != no_threads);//be sure all the threads finish the assignment

	qsort(my_data,my_n,sizeof(int),comp); //sort in bucket
	// 让线程依次执行，即：０，１，２，３　... 这样按照线程号顺序执行
	while(thread != my_rank);
	for(i = 0;i < my_n;i++) //copy back to data
		data[count++] = my_data[i];
	thread++;
}
