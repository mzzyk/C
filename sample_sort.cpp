#include <pthread.h>
#include <iostream>
#include <stdlib.h>    //qsort() 在这个头文件里
#include <fstream>
#include <string.h>
#include <semaphore.h>

using namespace std;

int data[100000];
int *sample_data;
int sample_cout=0;
int b_size;  //the size of bucker
int no_threads;   //the number of pthread

//int counter;  //the number of data
const int MAX = 32;     // Maximum number of threads allowed

int counter = 0;        // variable to be incremented by each thread
sem_t semaphore;	      // global semaphore, used for mutual exclusion
pthread_t tid[ MAX ];   // array of thread identifiers

//比较函数，升序
int cmp ( const void *a , const void *b )
{
    return *(int *)a - *(int *)b;
}
//将文件中的数据读取到数组里，并返回数组的大小
int read_file(int * seq, const char * filename)
{
    ifstream f1(filename);
    if(!f1)
        cout<<"cannot open file!"<<endl;
    int temp, count=0;
    while(f1>>temp)
    {
        seq[count]=temp;
        count++;
    }
    f1.close();
    //调用系统的快速排序函数
    //  qsort(seq,count,sizeof(int),cmp);
    return count;
}
//从给定数据中获取n个样本
int * get_sample(int * seq,int seq_size,int n)
{
    int k=no_threads;

    //  cout<<seq_size<<endl;
    qsort(seq,seq_size,sizeof(int),cmp);

    int splitter;
    int *result=new int[n];

    if(seq_size%k==0)
        splitter=seq_size/k;
    else
        splitter=seq_size/k+1;

    for(int i=1; i<no_threads; i++)
    {
        result[i-1]=seq[i*splitter-1];
    }

    return result;
}
void *increment( void *arg )
{
    long id = (long) arg;
    //  cout << "Thread " << id << " is starting!" << endl;
    int *bucket;
    //sem_wait( &semaphore );
    int sum=b_size;

    if(id == no_threads-1)
    {
        sum= counter - b_size* id;
        bucket=new int[sum];
    }
    else
        bucket=new int[b_size];
    // cout<<" bucket size is :"<<endl;
    for(int i=0; i<sum; i++)
    {
        bucket[i]=data[b_size*id+i];
    }
    int * sample0;
    sample0=get_sample(bucket,sum,no_threads-1);

    int sam_s=no_threads-1;
    sem_wait( &semaphore );
    for(int i=0 ; i<sam_s ; i++)
    {
        sample_data[sample_cout++]=sample0[i];
    }
    sem_post( &semaphore );

    return NULL;
}
int main()
{
    int chooser;
   const char *filename;
    cout<<"Please choose (1. K=4 and DATA) or (2. k=10 and  DATA2)"<<  endl;

     cout<<"Please input 1 or 2:"<<endl;
    cin>>chooser;

    while(cin.fail() || (chooser!=1 && chooser!=2) )  //cin输入错误时执行下边语句
    {
        cin.clear(); //清除流标记
        cin.sync();  //清空流
        cout<<"Please input 1 or 2:"<<endl; //打印错误提示
        cin>>chooser;
    }
     if(chooser ==1)
        {
            no_threads=4;
            string temp="DATA1";
            filename=temp.c_str();
            cout<<"The result of DATA1 is:"<<endl;

        }
        if(chooser==2)
        {
            no_threads=10;
           string temp="DATA2";
            filename=temp.c_str();
            cout<<"The result of DATA2 is:"<<endl;
        }


    sample_data=new int[no_threads * (no_threads-1)];

    //　read file
    counter=read_file(data,filename);

    //get the size of bucket
    if(counter%no_threads==0)
        b_size=counter/no_threads;
    else
        b_size=counter/no_threads+1;

    sem_init( &semaphore, 0, 1 );

    for( int i = 0; i < no_threads; i++ )
        pthread_create( &tid[ i ], NULL, increment, (void *) i );

    for(int  i = 0; i < no_threads; i++ )
        pthread_join( tid[ i ], NULL );


    int *k_sam=get_sample(sample_data,no_threads *(no_threads-1),no_threads);
    int **bucket;
    int *num=new int [no_threads];
    for(int i=0; i<no_threads; i++)
    {
        num[i]=0;
    }
    bucket = new int *[no_threads];
    for(int i=0 ; i<no_threads ; i++)
    {
        bucket[i]=new int[2*counter/no_threads];
    }
    // data to bucket by splitters
    for(int i=0 ; i<counter; i++)
    {
        if(data[i]<=k_sam[0] && data[i]>=0)
            bucket[0][num[0]++]=data[i];
        for(int j=1; j< no_threads-1; j++)
        {
            if(data[i]<=k_sam[j] && data[i] > k_sam[j-1])
                bucket[j][num[j]++]=data[i];
        }
        if(data[i]>k_sam[no_threads-2])
            bucket[no_threads-1][num[no_threads-1]++]=data[i];
    }
    //将各个bucket　中的数据进行排序
    int *result=new int[counter];
    for(int i=0 ; i<no_threads ; i++)
    {
        qsort(bucket[i],num[i],sizeof(int),cmp);
    }
    //用各个bucker中的数据组合在一起放在数组result 中
    int m=0;
    for (int i=0; i<no_threads; i++)
    {
        for(int j=0; j<num[i]; j++)
        {

            result[m++]=bucket[i][j];
        }
    }

    cout<<"the smallest one is:     "<<result[0]<<endl;
    cout<<"the n/4-th smallest one is :"<<result[counter/4]<<endl;
    cout<<"the n/2-th smallest one is: " <<result[counter/2]<<endl;
    cout<<"the largest one is :     "<<result[counter-1]<<endl;

    return 0;
}
