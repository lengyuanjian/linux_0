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
void init_list(fifo_list * list)
{
    list->head = list->tatil = nullptr;
}

void clear_list(fifo_list * list)
{
    list_node * node = list->head;
    while(node)
    {
        list_node * temp = node;
        node = node->next;
        free(temp);
    }
    list->head = list->tatil = nullptr;
}

list_node * push_back(fifo_list * list, T  data)
{
    list_node *node = (list_node *)malloc(sizeof(list_node));
    node->data = data;
    node->next = nullptr;
    if(list->head == nullptr)
    {
        list->head = list->tatil = node;
    }
    else
    {
        list->tatil->next = node;
        list->tatil = list->tatil->next;
    }
}

list_node * pop_front(fifo_list * list)
{
    if(list->head == nullptr)
    {
        return nullptr;
    }
    else
    {
        list_node * node = list->head;
        list->head = list->head->next;
        return node;
    }
}

bool is_empty(fifo_list * list)
{
    return (list->head == nullptr) ? true : false;
}
#include <assert.h>
int main(int argc, char * argv[]) 
{
    std::cout << argv[0] <<std::endl;
    
    {
        fifo_list my_list;
        init_list(&my_list);
        assert(my_list.head == nullptr);
        assert(my_list.tatil == nullptr);

    }

    {
        fifo_list my_list;
        init_list(&my_list);
        T data1 = 10;
        push_back(&my_list, data1);
        assert(my_list.head != nullptr);
        assert(my_list.tatil != nullptr);
        assert(my_list.head->data == data1);

    }
    {
        fifo_list my_list;
        init_list(&my_list);
        T data1 = 10;
        push_back(&my_list, data1);
        list_node *popped_node = pop_front(&my_list);
        assert(popped_node != nullptr);
        assert(popped_node->data == data1);
        assert(my_list.head == nullptr);

    }
    {
        fifo_list my_list;
        init_list(&my_list);
        assert(is_empty(&my_list));
        T data1 = 10;
        push_back(&my_list, data1);
        assert(!is_empty(&my_list));

    }
    {
        fifo_list my_list;
        init_list(&my_list);
        T data1 = 10;
        push_back(&my_list, data1);
        clear_list(&my_list);
        assert(my_list.head == nullptr);
        assert(my_list.tatil == nullptr);

    }
    return argc;
}
