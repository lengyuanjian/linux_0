#include <iostream>

/*
1 节点颜色：每个节点都被标记为红色或黑色。
2 根节点颜色：根节点是黑色的。
3 叶子节点颜色：叶子节点（NIL节点，即空节点）是黑色的。
4 红色节点规则：如果一个节点是红色的，则其两个子节点都必须是黑色的。
5 路径规则：从任一节点到其每个叶子节点的所有路径都必须包含相同数量的黑色节点。
  这个性质保证了红黑树的平衡性，因为它限制了任何一条路径相对于其他路径的长度差异，使得最长路径不超过最短路径的两倍。
*/

typedef enum { RED, BLACK } Color;

typedef struct RBTreeNode {
    int key;
    Color color;
    struct RBTreeNode *left, *right, *parent;
} RBTreeNode;

int main(int argc, char * argv[]) 
{
    std::cout << argv[0] <<std::endl;
    

    return argc;
}
