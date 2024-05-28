#include <iostream>

// A B D H I E J K C F L M G N O 
// H D I B J E K A L F M C N G O 
// H I D J K E B L M F N O G C A 
// A B C D E F G H I J K L M N O 
struct tree_node
{
    int                key{0};
    char               data{'*'}; 
    tree_node *        p{nullptr};
    tree_node *        l{nullptr};
    tree_node *        r{nullptr};
};

struct bst_tree
{
    tree_node * root{nullptr};
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

tree_node * create_node(int key, char value, tree_node * p, tree_node * l, tree_node *r)
{
    tree_node * node = (tree_node *)malloc(sizeof(tree_node));
    node->key = key;
    node->data = value; 
    node->p = p;
    node->l = l;
    node->r = r;
    return node;
}

void release_node(tree_node * node)
{
    if(node)
    {
        free(node);
    }
}
      
tree_node * insert_node(tree_node * node, int key, char value)
{
    if(node == nullptr)
    {
        return create_node(key, value, nullptr, nullptr, nullptr);
    }
    if(node->key == key)
    {
        node->data = value;
        return node;
    }
    else if(node->key > key)
    {
        if(node->l == nullptr)
        {
            return node->l = create_node(key, value, node, nullptr, nullptr); 
        }
        else
        {
            return insert_node(node->l, key, value);
        }
    }
    else /*if(node->key < key)*/
    {
        if(node->r == nullptr)
        {
            return node->r = create_node(key, value, node, nullptr, nullptr);
        }
        else
        {
            return insert_node(node->r, key, value);
        }
    }
}

tree_node * bst_tree_insert(bst_tree * tree, int key, char value)
{
    if(tree->root == nullptr)
    {
        tree->root = create_node(key, value);
        return tree->root;
    }
    
    return insert_node(tree->root, key, value);
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
    else /*if(root->key < key)*/
    {
        return find_node(root->r, key);
    } 
}

tree_node * bst_find_node(bst_tree * tree, int key)
{
    if(tree->root == nullptr)
    {
        return nullptr;
    }
    
    return find_node(tree->root, key);
}

tree_node * delete_node(tree_node * root, int key)
{
    if(root == nullptr)
    {
        return nullptr; // 未找到键
    }
    else if(root->key == key)
    {
        // 情况1：要删除的节点没有子节点
        if(root->l == nullptr && root->r == nullptr)
        {
            release_node(root);
            return nullptr;
        }
        // 情况2：要删除的节点只有左子节点
        else if(root->l != nullptr && root->r == nullptr)
        {
            tree_node * node = delete_node(root->l, key);
            release_node(root);
            return node;
        }
        // 情况3：要删除的节点只有右子节点
        else if(root->l == nullptr && root->r != nullptr)
        {
            tree_node * node = delete_node(root->r, root->r->key);
            release_node(root);
            return node;
        }
        // 情况4：要删除的节点既有左子节点又有右子节点
        else
        {
            tree_node * node = delete_node(root->r, root->r->key);
            release_node(root);
            return node;
        }
    }
    else if(root->key > key)
    {
        root->l = delete_node(root->l, key);
        return root;
    }
    else // if(root->key < key)
    {
        root->r = delete_node(root->r, key);
        return root;
    } 
}

void release_tree(tree_node *root)
{
    if (root != nullptr)
    {    
        release_tree(root->l);
        release_tree(root->r); 
        release_node(root);
    }
}

void bst_release_tree(bst_tree * tree)
{
    if(tree->root != nullptr)
    {
        release_tree(tree->root);
        tree->root = nullptr;
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
    bst_tree  _tree ={}; 
    bst_tree * tree = &_tree;
    tree_node * node = nullptr;
    node = bst_tree_insert(tree, 50, 'A');std::cout<< node->key << ":" << node->data <<"\n";
    node = bst_tree_insert(tree, 30, 'B');std::cout<< node->key << ":" << node->data <<"\n";
    node = bst_tree_insert(tree, 80, 'C');std::cout<< node->key << ":" << node->data <<"\n";
    node = bst_tree_insert(tree, 20, 'D');std::cout<< node->key << ":" << node->data <<"\n";
    node = bst_tree_insert(tree, 40, 'E');std::cout<< node->key << ":" << node->data <<"\n";
    node = bst_tree_insert(tree, 70, 'F');std::cout<< node->key << ":" << node->data <<"\n";
    node = bst_tree_insert(tree, 90, 'G');std::cout<< node->key << ":" << node->data <<"\n";
    node = bst_tree_insert(tree, 19, 'H');std::cout<< node->key << ":" << node->data <<"\n";
    node = bst_tree_insert(tree, 21, 'I');std::cout<< node->key << ":" << node->data <<"\n";
    node = bst_tree_insert(tree, 31, 'J');std::cout<< node->key << ":" << node->data <<"\n";
    node = bst_tree_insert(tree, 41, 'K');std::cout<< node->key << ":" << node->data <<"\n";
    node = bst_tree_insert(tree, 51, 'L');std::cout<< node->key << ":" << node->data <<"\n";
    node = bst_tree_insert(tree, 71, 'M');std::cout<< node->key << ":" << node->data <<"\n";
    node = bst_tree_insert(tree, 81, 'N');std::cout<< node->key << ":" << node->data <<"\n";
    node = bst_tree_insert(tree, 91, 'O');std::cout<< node->key << ":" << node->data <<"\n";
    // char arry[] = {50,30,80,20,40,70,90,19,21,31,41,51,71,81,91};
    // for(auto key : arry)
    // {
    //     tree_node * node = find_node(tree->root, key);
    //     if(node)
    //     {
    //         std::cout<< ((node->p != nullptr) ? node->p->data : '-') << "\n";
    //         std::cout<< node->key << ":" << node->data <<"\n";
    //         std::cout<< ((node->l != nullptr) ? node->l->data : '-') << "  " << ((node->r != nullptr) ? node->r->data : '-') <<"\n";
    //     }
    //     else
    //     {
    //         std::cout<<"err find\n";
    //     }

    // }
    preface_traversal(tree->root);
    std::cout<<"\n";
    inorder_traversal(tree->root);
    std::cout<<"\n";
    postorder_traversal(tree->root);
    std::cout<<"\n";
    level_order_traversal(tree->root);
    std::cout<<"\n";
    // level_order_traversal1(tree->root);
    // std::cout<<"\n";
    std::cout<<"-----------------------------\n"; 
     
    bst_release_tree(tree);

    return argc;
}
