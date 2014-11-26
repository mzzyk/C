#include <pthread.h>
#include <iostream>
#include <stdlib.h>    //qsort() 在这个头文件里
#include <fstream>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>

using namespace std;

int data[100000];

char **input;
char **output;

const int b_size=10;  //the size of bucker
int count_bit[b_size];
int offset[b_size];

int no_threads;   //the number of pthread
const int radix_size=6;

int counter;  //the number of data
const int MAX = 32;     // Maximum number of threads allowed

sem_t sort_sem;
sem_t count_sem;	      // global semaphore, used for mutual exclusion
pthread_t tid[ MAX ];   // array of thread identifiers
pthread_barrier_t barrier;
pthread_mutex_t my_lock;

//比较函数，升序
int cmp ( const void *a , const void *b )
{
    return *(int *)a - *(int *)b;
}
//将文件中的数据读取到数组里，并返回数组的大小
int read_file(int * seq, char * filename)
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

    return count;
}
//int to char[6]
void intTochar()
{
    input=new char *[counter];

    for(int i=0; i<counter; i++)
    {
        input[i]=new char[radix_size];
        memset(input[i],0,radix_size*sizeof(char));
    }

    for(int i=0; i<counter; i++)
    {
        //将给定数据格式化成六位，高位补零
        sprintf(input[i],"%06d",data[i]);
    }

}

//统计key的个数，并将每个pthread统计的结果写入总的counter_bit[];
void radix (int index,  int first, int last,int thread_id)
{
    int loc_counter[b_size];
    memset(loc_counter,0,b_size*sizeof(int));

    for( int i = first; i < last; ++i)
    {
        char c = input[i][index];
        //将字符强制转换成对应的整形数
        int n=c-48;
        loc_counter[n]++;
    }
    //同一时间，只能一个pthread进行couter_bit 写
    sem_wait(&count_sem);
    for(int i=0; i<b_size; i++)
    {
        count_bit[i]+=loc_counter[i];
    }
    sem_post(&count_sem);



}
//通过全局的ounter_bit 来计算每一个ey的位置偏移（offset）
void global_count()
{
    sem_wait(&count_sem);
    //初始化offset数组
    memset(offset,0,b_size*sizeof(int));

    offset[0]=0;
    for( int i = 1; i < b_size; ++i)
    {
        offset[i] = offset[i-1] + count_bit[i-1];
    }

    sem_post(&count_sem);

}
//对nput进行指定radix 位置上的排序，并将结果存入output中；
void sort_input(int index,int id)
{

    for( int i=0; i<counter; i++)
    {
        char c = input[i][index];

        sem_wait(&count_sem);
        int current_offset = offset[c-48]++;
        output[current_offset] = input[i];
        sem_post(&count_sem);
    }

}

void *thread_do_count( void *arg )
{
    //获取线程 id
    long id = (long) arg;
    //每一个线程处理数据块的大小。
    int block=counter/no_threads;
    int first_idx=id*block;   //包括索引为这个值的数值。
    int last_idx=counter-(no_threads-id-1)*block;  //不包括索引为这个值的数值。
    for(int i = radix_size-1; i >= 0; --i)
    {
        radix(i, first_idx, last_idx,id);

        //让所有线程都执行在此步骤后再往下执行
        pthread_barrier_wait(&barrier);
        global_count();

        pthread_barrier_wait(&barrier);

        if(id==0)
        {
            sort_input(i,id);
            char ** temp = input;
            input = output;
            output = temp;
        }

        memset(count_bit,0,b_size*sizeof(int));
        pthread_barrier_wait(&barrier);
    }
    return NULL;
}
int main()
{
    int chooser;
    char filename[100];
    cout<<"############ Parallel algorithm for RadixSort ############### "<<endl;
    cout<<endl;
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
        strcpy(filename,temp.c_str());
        cout<<"The result of DATA1 is:"<<endl;

    }
    if(chooser==2)
    {
        no_threads=10;
        string temp="DATA2";
        strcpy(filename,temp.c_str());
        cout<<"The result of DATA2 is:"<<endl;
    }

    counter=read_file(data,filename);
    intTochar();

    output=new char *[counter];

    for(int i=0; i<counter; i++)
    {
        output[i]=new char[radix_size];
        memset(output[i],0,radix_size*sizeof(char));
    }

    memset(count_bit,0,b_size*sizeof(int));

    memset(offset,0,b_size*sizeof(int));

    sem_init( &count_sem, 0, 1 );

    pthread_barrier_init(&barrier,NULL,no_threads);

    for( int i = 0; i < no_threads; i++ )
        pthread_create( &tid[ i ], NULL, thread_do_count, (void *) i );

    for(int  i = 0; i < no_threads; i++ )
        pthread_join( tid[ i ], NULL );

    pthread_barrier_destroy(&barrier);

    cout<<"the smallest one is:     "<<atoi(input[0])<<endl;
    cout<<"the n/4-th smallest one is :"<<atoi(input[counter/4])<<endl;
    cout<<"the n/2-th smallest one is: " <<atoi(input[counter/2])<<endl;
    cout<<"the largest one is :     "<<atoi(input[counter-1])<<endl;

    return 0;
}
