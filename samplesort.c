//this is an example of sample_sort by MPI  on linux (clusternode 4)
//compile command :mpicc -o samplesort samplesort.c 
//run command     :mpirun -np 4 ./samplesort DATA1
//run command for local: mpiexec.hydra -f /home/zyk/hostfile -n 4 ./samplesort DATA1
//#4 is the number of process ;DATA1 is file of data to test
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include <mpi.h>
#define  MAX 32
#define MAX_SIZE 100000

int n;   //the number of data
int no_process;// the number of threads
int data[MAX_SIZE];//DATA
int temp[MAX_SIZE];
int count;
int thread;
int number;
int root=0; //root processor

int *sample;
int *splitter;
int *bucket_size;

void sample_sort(int rank);
int comp(const void *a,const void *b)
{
	return *((int *)a) - *((int *)b);
}
int main(int argc, char * argv[])
{
	int i;
	char *c;
        
	FILE *fp;
	if((fp = fopen(argv[1],"r")) == NULL)
	{ 
		printf("can't open this file!");
		exit(0);
	}
	
	i = 0;
	while(fscanf(fp,"%d",&data[i]) != EOF) i++;
	n = i;
	int rank;	
	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	MPI_Comm_size(MPI_COMM_WORLD,&no_process);
	sample_sort(rank);
	if(rank == root)
	{
		printf("the smallest number is %d\n",data[0]);
		printf("the 1/4 samllest number is %d\n",data[n/4]);
		printf("the 1/2 samllest number is %d\n",data[n/2]);
		printf("the largest number is %d\n",data[n-1]);
	}
	MPI_Finalize();
	return 0;
}

void sample_sort(int rank)  
{
	int my_rank = rank;

	splitter = (int *)malloc((no_process - 1) * sizeof(int));
	sample = (int *)malloc(2*(no_process - 1) * no_process * sizeof(int));
	bucket_size = (int *)malloc(no_process*sizeof(int));
	
	int my_n = n/no_process;
	int my_first_i;
	int my_last_i;
	int space,size;
	int i,j;
	int *my_data = (int *)malloc(((3 * n)/(2*no_process)) * sizeof(int));
	my_first_i = my_rank * (n/no_process);
	
	if(my_rank == no_process-1)
		my_last_i = n - 1;
	else
		my_last_i = my_first_i + my_n - 1;
	
	size=my_last_i - my_first_i + 1;
	space = (my_last_i - my_first_i + 1)/(2*no_process);	

	//evenly assign the numbers to every bucket
	for(i = 0,j = my_first_i;i < my_n,j < my_last_i;i++,j++) 
		my_data[i] = data[j];
	
	int *loc_sample=malloc((2*no_process-1)*sizeof(int));
	int loc_sample_size=2*no_process-1;

	//sort in bucke
	qsort(my_data, size, sizeof(int), comp); 

	 //select samples
	for(i = 1; i < size; i++)
		if(i%space == 0)
			loc_sample[i/space-1]=my_data[i];

	MPI_Gather(loc_sample,loc_sample_size,MPI_INT,sample,loc_sample_size,MPI_INT,root,MPI_COMM_WORLD);

	if(my_rank == root){

		int sum=no_process*(2*no_process-1);
		
		//sort samples
		qsort(sample,sum,sizeof(int),comp);
		space = 2 * no_process-1;
		
		// select splitters
		for( i = 0; i < no_process-1; i++) 
		{
		splitter[i] = sample[(i+1)*space];
		}	

	}	
	MPI_Bcast(splitter,no_process-1,MPI_INT,root,MPI_COMM_WORLD);
	MPI_Barrier(MPI_COMM_WORLD);

	//assign numbers by splitters
	if(my_rank == root){ 
		for(i = 0, j = 0; i < n; i++)
			if(data[i] <= splitter[0])
				my_data[j++] = data[i];
		my_n = j;
		bucket_size[my_rank] = my_n;

	}
	else if(my_rank == (no_process-1))
	{
		for(i = 0,j = 0; i < n; i++)
			if(data[i] > splitter[no_process-2])
				my_data[j++] = data[i];
		my_n = j;
		bucket_size[my_rank] = my_n;

	}

	else{
		for(i = 0,j = 0;i < n;i++)
			if(data[i] <= splitter[my_rank] && data[i] > splitter[my_rank-1])
				my_data[j++] = data[i];
		my_n = j;
		bucket_size[my_rank] = my_n;
	
	}

	qsort(my_data,bucket_size[my_rank],sizeof(int),comp); //sort in bucket
	
	int *loc_bucket=malloc(no_process*sizeof(int));
	memset(loc_bucket,0,no_process);
	MPI_Allgather(&my_n,1,MPI_INT,bucket_size,1,MPI_INT,MPI_COMM_WORLD);

	int *steps=malloc(no_process*sizeof(int));

	steps[0]=0;
	for(i=1;i<no_process;i++)
	{
		steps[i]=steps[i-1]+bucket_size[i-1];
	}
	/*函数功能描述：收集各个进程（包括自己）发送来的数据，依次按照进程号顺序
	 * 和指定的偏移存放在根进程给定首地址的数组中
	 * 参数一：要发送数据的首地址
	 * 参数二：要发送数据的个数
	 * 参数三：要发送数据的数据类型
	 * 参数四：接收数据的首地址
	 * 参数五：各个process要发送数据个数组成的数组
	 * 参数六：对应与参数五中数据的偏移
	 * 参数七：接收数据类型
	 * 参数八：接收数据的进程号（这里的root表示根进程收集各个进程发送来的数据）
	 * */
	MPI_Gatherv(my_data,bucket_size[rank],MPI_INT,data,bucket_size,steps,MPI_INT,0,MPI_COMM_WORLD);
}
