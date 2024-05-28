#include <iostream>
/*
//           A5  
//        /     \
//       B3      C8
//      / \     /  \
//     D2  E4  F7   G9

*/



// 前序遍历：A B D E C F G 
// 中序遍历：D B E A F C G 
// 后序遍历：D E B F G C A
// 层序遍历：
struct tree_node
{
    int                key{0};
    char               data{'*'};
    tree_node *        p{nullptr};
    tree_node *        l{nullptr};
    tree_node *        r{nullptr};
};

tree_node * create_node(int key, char value)
{
    tree_node * node = (tree_node *)malloc(sizeof(tree_node));
    node->key = key;
    node->data = value;
    node->p = nullptr;
    node->l = nullptr;
    node->r = nullptr;
    return node;
}

void release_node(tree_node * node)
{
    if(node)
    {
        free(node);
    }
}

tree_node * insert_node(tree_node * root, int key, char value)
{
    if(root == nullptr)
    {
        root = create_node(key, value);
        root->key = key;
        root->data = value;
        return root;
    }
    if(root->key == key)
    {
        root->data = value;
        return root;
    }
    else if(root->key > key)
    {
        tree_node * node = insert_node(root->l, key, value);
        if(root->l == nullptr)
        {
            root->l = node;
            node->p = root;
        }
    }
    else if(root->key < key)
    {
        tree_node * node = insert_node(root->r, key, value);
        if(root->r == nullptr)
        {
            root->r = node;
            node->p = root;
        }
    }
    return root;
}

tree_node * find_node(tree_node * root, int key)
{
    if(root == nullptr || root->key == key)
    {
        return root;
    }
    else if(root->key > key)
    {
        return find_node(root->l, key);
    }
    else if(root->key < key)
    {
        return find_node(root->r, key);
    } 
}

// tree_node * delete_node(tree_node * root, int key)
// {
//     if(root == nullptr)
//     {
//         return root;
//     } 
//     else if(root->key == key)
//     {
//         return root;
//     }
//     else if(root->key > key)
//     {
//         tree_node * node = delete_node(root->l, key);
//         return nullptr;
//     }
//     else if(root->key < key)
//     {
//         tree_node * node = delete_node(root->r, key);
//         return nullptr;
//     } 
// }

void release_tree(tree_node *root)
{
    if (root != nullptr)
    {    
        release_tree(root->l);
        release_tree(root->r); 
        release_node(root);
    }
}

// preorder traversal, inorder traversal and postorder traversal.
// 根-左-右
void preface_traversal(tree_node *root)
{
     if(root != nullptr)
     {
        std::cout<< root->data << " ";
        preface_traversal(root->l);
        preface_traversal(root->r);
     }
}
// 左-根-右
void inorder_traversal(tree_node *root)
{
     if(root != nullptr)
     {
        inorder_traversal(root->l);
        std::cout<< root->data << " ";
        inorder_traversal(root->r);
     }
}
// 左-右-根
void postorder_traversal(tree_node *root)
{
     if(root != nullptr)
     {
        postorder_traversal(root->l);
        postorder_traversal(root->r);
        std::cout<< root->data << " ";
     }
}

// 层序遍历
void level_order_traversal(tree_node *root)
{
    struct  queue_node
    {
        tree_node *data;
        queue_node *next;
    };
    struct fifo_queue
    {
        queue_node * front;
        queue_node * rear;
    }queue = {nullptr, nullptr};

    queue.front = (queue_node *)malloc(sizeof(queue_node));
    queue.rear  = (queue_node *)malloc(sizeof(queue_node));
    queue.front->data = root;
    queue.front->next = queue.rear;
    queue.rear->data = nullptr;
    queue.rear->next = nullptr;

    while(queue.front != nullptr)
    {   
        queue_node * temp = queue.front;
        tree_node *tree_node = queue.front->data;
        queue.front = queue.front->next;

        if(tree_node != nullptr)
        {
            std::cout<< tree_node->data << " ";
            if(tree_node->l != nullptr)
            {
                queue.rear->data = tree_node->l;
                queue.rear->next = (queue_node *)malloc(sizeof(queue_node));
                queue.rear = queue.rear->next;
                queue.rear->next = nullptr;
                queue.rear->data = nullptr;
            }
            if(tree_node->r != nullptr)
            {
                queue.rear->data = tree_node->r;
                queue.rear->next = (queue_node *)malloc(sizeof(queue_node));
                queue.rear = queue.rear->next;
                queue.rear->next = nullptr;
                queue.rear->data = nullptr;
            } 
        }
        free(temp);
    }
}

// 有问题版本
void level_order_traversal1(tree_node *root)
{
    struct  queue_node
    {
        tree_node *data;
        queue_node *next;

    };
    struct fifo_queue
    {
        queue_node * front;
        queue_node * rear;
    }queue = {nullptr, nullptr};

    if (root == nullptr)
        return;

    queue.front = (queue_node *)malloc(sizeof(queue_node));
    queue.front->data = root;
    queue.front->next = nullptr;
    queue.rear  = queue.front;

    while(queue.front != nullptr)
    {   
        queue_node * temp = queue.front;
        tree_node *tree_node = queue.front->data;
        queue.front = queue.front->next;

        if(tree_node != nullptr)
        {
            std::cout<< tree_node->data << " ";

            if(tree_node->l != nullptr)
            {
                queue.rear->next = (queue_node *)malloc(sizeof(queue_node));
                queue.rear = queue.rear->next;
                queue.rear->data = tree_node->l;
                queue.rear->next = nullptr;
            }
            if(tree_node->r != nullptr)
            {
                queue.rear->next = (queue_node *)malloc(sizeof(queue_node));
                queue.rear = queue.rear->next;
                queue.rear->data = tree_node->r;
                queue.rear->next = nullptr;
            }
        }
        free(temp);
    }
}
int main(int argc, char * argv[]) 
{
    std::cout << argv[0] <<std::endl;
    tree_node * root = nullptr;
    root = insert_node(root, 5, 'A');
    insert_node(root, 3, 'B');
    insert_node(root, 8, 'C');
    insert_node(root, 2, 'D');
    insert_node(root, 4, 'E');
    insert_node(root, 7, 'F');
    insert_node(root, 9, 'G');
    insert_node(root, 6, '*');
    tree_node * find = find_node(root, 9);
    if(find)
    {
        std::cout<<find->data<<"\n";
    }
    preface_traversal(root);
    std::cout<<"\n";
    inorder_traversal(root);
    std::cout<<"\n";
    postorder_traversal(root);
    std::cout<<"\n";
    level_order_traversal(root);
    std::cout<<"\n";
    level_order_traversal1(root);
    std::cout<<"\n";
    release_tree(root);

    return argc;
}
