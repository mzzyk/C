#include <stdio.h>
#include <stdlib.h>
//数据结构练习
//结构体和列表。注意指针中‘->’和‘.’的区别（如果是指针要用‘->’,才可以正确访问）
typedef struct list_element
{
    int data;
    struct list_element *next;
} list_element; //定义结构体的别名
list_element *head,*taill;
int main()
{
    list_element * l;
    l=(list_element*)malloc(sizeof(list_element));

    l->data=6;
    l->next=NULL;
    int i=0,j;
    list_element  *list[10];
    for(j=0; j<10; j++)
    {
        list[j]=malloc(sizeof(list_element));
    }
    head=list[0];
    taill=list[9];
    for(i=0; i<10; i++)
    {
        list[i]->data=i;
        if(i<9)
            list[i]->next=list[i+1];
        else
            list[i]->next=NULL;

    }
    list_element * tail;
    tail=malloc(sizeof(list_element));
    tail=list[0];
    while(1)
    {
        if(tail->data==6)
        {
            list_element *temp=tail->next;
            tail->next=l;
            l->next=temp;
            break;
        }
        tail=tail->next;
    }
    tail=head;
    if(tail->data==9)
    {
        head=tail->next;
        free(tail);
    }
    else
    {
        while(1)
        {
            if(tail->next->data==9)
            {
                if(tail->next==NULL)
                {
                    taill=tail;
                    free(tail->next);
                    break;
                }
                else
                {
                    list_element *tem=tail->next;
                    tail->next=tail->next->next;
                    free(tem);
                    break;
                }
            }
            tail=tail->next;

        }
    }

    //free(tail);
    list_element *first;
    first=(list_element*)malloc(sizeof(list_element));
    first=head;

    printf("first is :%d\n",first->next->data);
    while(1)
    {
        printf("%d->",first->data);
        if(first->next==NULL)
            break;
        first=first->next;
    }
    printf("\nHello world!\n");
    free(first);
    free(list[0]);
    return 0;

}
