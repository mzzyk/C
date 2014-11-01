#include <iostream>
#include <stdlib.h>
#include<unistd.h>
#include <fstream>

using namespace std;
//定义数组
int seq1[3000];
int seq2[3000];
//定义缓存空间
char s[10];
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
    char ch;
    while(f1>>temp)
    {
        seq[count]=temp;
        f1>>ch;
        count++;
    }
    f1.close();
    //调用系统的快速排序函数
    qsort(seq,count,sizeof(int),cmp);
    return count;
}
int main()
{
    int fd[2][2];
    int pid;
    int i,j,min_num;
    pipe(fd[0]);
    pipe(fd[1]);

    int s1=read_file(seq1,"file/seq2.txt");
    int s2=read_file(seq2,"file/seq1.txt");
    if(s1<s2)
        min_num=s1;
    else
        min_num=s2;

    pid=fork();
    for(i=0; i<min_num; i++)
    {
        if(0!=pid)
        {
            sprintf(s,"%d",seq1[s1-1]);
            write(fd[0][1],s,4);
            read(fd[1][0],s,4);
            int temp0=atoi(s);
            seq1[s1-1]=temp0;
            qsort(seq1,s1,sizeof(int),cmp);
            //    printf("child read is %s\n",s);
            if(i==min_num-1)
            {
                //输出Seq1排序后的结果
                printf("the sequence of parent is:\n");
                for( i=0; i<s1; i++)
                {
                    cout<<seq1[i]<<" ";
                }
                cout<<endl;
            }
        }
        else
        {
            read(fd[0][0],s,4);
            int temp1=atoi(s);
            seq2[0]=temp1;
            qsort(seq2,s2,sizeof(int),cmp);
            //      printf("parent reading is %d\n",temp);
            if(i==min_num-1)
            {
                //输出Seq2排序后的结果
                printf("the sequence of child is:\n");
                for(j=0; j<s2; j++)
                {
                    cout<<seq2[j]<<" ";
                }
                cout<<endl;
            }


            sprintf(s,"%d",seq2[0]);
            write(fd[1][1],s,4);

        }
    }
    return 0;
}
