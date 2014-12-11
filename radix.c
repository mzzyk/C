/*程序功能描述如下：
 * 利用MPI　对基数排序进行实现
 *　编译命令：  mpicc -o radix radix.c
 *　执行命令：  mpiexec.hydra -f /home/zyk/hostfile -n 4 ./radix DATA2
 *	其中DATA2 为测试文件，里面有10000个int 型数据
 *	４　为进程数目（或者是集群节点数目）；
 * */
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX 32
#define MAX_SIZE 100000
#define MAX_BUCKET 10

int n;//the number of data
int data[MAX_SIZE];
int temp[MAX_SIZE];
int bucket[MAX_BUCKET];
int k;//the number of threads
int max_num;
int pow_10=1;

void get_max_num();
void scan();
void radix_sort(int rank);

int main(int argc,char *argv[]){
	FILE *fp;
	int i=0;
	if((fp=fopen(argv[1],"r"))==NULL){
		printf("can't open the file!");
		exit(0);
	}
	while(fscanf(fp,"%d",&data[i])!=EOF)
		i++;
	n = i;

	get_max_num();

	int rank;
	MPI_Init(&argc, &argv);
	// get the id of process
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	radix_sort(rank);

	if(rank == 0){
		printf("the smallest number is: %d\n",data[0]);
		printf("the n/4 smallest number is: %d\n",data[n/4]);
		printf("the n/2 smallest number is: %d\n",data[n/2]);
		printf("the largest number is: %d\n",data[n - 1]);

	}

	MPI_Finalize();
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

void radix_sort(int rank)
{  //radix_sort
	int my_rank =rank;

	MPI_Comm_size(MPI_COMM_WORLD,&k);

	int my_first = my_rank * (n / k);
	int my_last;
	int size;

	if(my_rank == k-1)
		my_last = n-1;
	else
		my_last = my_first + n/k -1;
	size = my_last - my_first + 1;
	int i;
	while((max_num / pow_10) > 0)
	{

		for(i = my_first; i <= my_last; i++)
		{
			bucket[(data[i] / pow_10) % MAX_BUCKET]++;   //get the sum 
		}
		MPI_Barrier(MPI_COMM_WORLD);
		int i;

		int rbucket[MAX_BUCKET];
		for( i=0;i<MAX_BUCKET;i++)
		{
			//将各个进程统计结果累加
			MPI_Reduce(&bucket[i],&rbucket[i],1,MPI_INT,MPI_SUM,0,MPI_COMM_WORLD);
		}
		if (rank == 0)
		{
			for(i=0;i<MAX_BUCKET;i++)
			{
				bucket[i]=rbucket[i];
			}
			scan();
            //基数排序
			for(i = (n - 1) ;i >= 0; i--) 
				temp[--bucket[(data[i] / pow_10) % MAX_BUCKET]] = data[i];

			for(i = 0; i < n; i++) //copy back
				data[i] = temp[i];
		}
		pow_10 *= 10;
		for(i = 0; i < MAX_BUCKET; i++)
			bucket[i] = 0; 

		MPI_Bcast(data, n, MPI_INT, 0, MPI_COMM_WORLD);
	}

}
