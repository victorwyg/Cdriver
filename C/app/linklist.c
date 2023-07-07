#include <stdio.h>
#include <stdlib.h>


typedef struct 
{
    int data;
    struct linklist *next;
    
}linklist;

void insert(linklist **head, int data)
{
    linklist *newNode = (linklist *)malloc(sizeof(linklist));
    newNode->data = data;
    newNode->next = NULL;

    if (*head == NULL)
    {
        // 如果链表为空，将新节点设为头节点
        *head = newNode;
    }
    else
    {
        // 找到链表的末尾节点
        linklist *current = *head;
        while (current->next != NULL)
        {
            current = current->next;
        }
        // 在末尾插入新节点
        current->next = newNode;
    }
}

// 打印链表中的所有节点数据
void printList(linklist *head)
{
    linklist *current = head;
    while (current != NULL)
    {
        printf("%d ", current->data);
        current = current->next;
    }
    printf("\n");
}

// 释放链表的内存空间
void freeList(linklist **head)
{
    linklist *current = *head;
    linklist *next;
    while (current != NULL)
    {
        next = current->next;
        free(current);
        current = next;
    }
    *head = NULL;
}

int main()
{
    linklist *head = NULL;  // 链表的头节点

    // 在链表末尾插入节点
    insert(&head, 14);
    insert(&head, 12);
    insert(&head, 18);

    // 打印链表数据
    printf("Linked List: ");
    printList(head);

    // 释放链表内存空间
    freeList(&head);

    return 0;
}