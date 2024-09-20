#include <stdio.h>
#include <stdlib.h> 
#include "sys_queue.h"


// 定义链表节点结构
struct entry 
{
    int value;
    LIST_ENTRY(entry) entries;
};

static LIST_HEAD(, entry) head = LIST_HEAD_INITIALIZER(&head);

struct entry *malloc_node()
{
    return (struct entry *)malloc(sizeof(struct entry));
}

int main() 
{ 
    // struct listhead head;
    // LIST_INIT(&head);
 

    for (int i = 0; i < 3; i++) 
    {
        // 分配内存给新节点
        struct entry *e = malloc_node();
        if (e == NULL) 
        {
            perror("malloc failed");
            return 1;
        }
        e->value = i + 1;
        LIST_INSERT_HEAD(&head, e, entries);
    }

    // 遍历链表并打印元素
    struct entry *e;
    printf("List contents:\n");
    LIST_FOREACH(e, &head, entries) 
    {
        printf("%d\n", e->value);
    }

    // 删除链表中的所有元素
    while (!LIST_EMPTY(&head)) 
    {
        e = LIST_FIRST(&head);
        // 从链表中移除元素
        LIST_REMOVE(e, entries);
        free(e);  // 释放内存
    }

    printf("LIST_HEAD\n");

    

    return 0;
}
