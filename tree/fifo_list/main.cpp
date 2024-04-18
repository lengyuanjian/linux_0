#include <iostream>

typedef int T;

struct list_node
{
    T  data;
    list_node * next;
};

struct fifo_list
{
    list_node * head;
    list_node * tatil;
};

bool is_empty(fifo_list * list)
{
    return (list->head == nullptr) ? true : false;
}

list_node * pop(fifo_list * list)
{
    list_node * node = list->head;
    list->head = list->head->next;
    return node;
}
void push(fifo_list * list, list_node * node)
{
    if(list->head == nullptr)
    {
        list->head = ;
    }
}

int main(int argc, char * argv[]) 
{
    std::cout << argv[0] <<std::endl;
   

    return argc;
}
