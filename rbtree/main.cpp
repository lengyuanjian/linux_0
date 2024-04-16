#include <iostream>


struct tree_node
{
    int                key{0};
    enum{red, black}   color{red};
    void *             data{nullptr};
    tree_node *        parent{nullptr};
    tree_node *        son_l{nullptr};
    tree_node *        son_r{nullptr};
};
tree_node * create_node()
{
    tree_node * node = (tree_node *)malloc(sizeof(tree_node));
    return node;
}

void release_node(tree_node * node)
{
    if(node)
    {
        free(node);
    }
}

tree_node * create_rb_tree()
{
    tree_node * node = create_node();
    return node;
}
void create_rb_tree(tree_node * root)
{
    
}

int main(int argc, char * argv[]) 
{
    std::cout << argv[0] <<std::endl;

    return argc;
}
