#include <iostream>
/*
//            A  
//         /     \
//       B         C
//      / \      /   \
//     D   E    F     G
//    / \   \    \
//   H   I   J    K
*/
// 前序遍历：ABDHIEJCFKG
// 中序遍历：HDIBEJAFKCG
// 后序遍历：HIDJEBKFGCA
// 层序遍历：ABCDEFGHIJK
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
    else if(root->key < key)
    {
        tree_node * node = insert_node(root->l, key, value);
        if(root->l == nullptr)
        {
            root->l = node;
        }
    }
    else if(root->key > key)
    {
        tree_node * node = insert_node(root->r, key, value);
        if(root->r == nullptr)
        {
            root->r = node;
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
    else if(root->key < key)
    {
        return find_node(root->l, key);
    }
    else if(root->key > key)
    {
        return find_node(root->r, key);
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
    tree_node * find = find_node(root, 9);
    if(find)
    {
        std::cout<<find->data<<"\n";
    }
    release_tree(root);

    return argc;
}
