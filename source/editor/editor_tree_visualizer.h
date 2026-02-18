#ifndef EDITOR_TREE_VISUALIZER_H
#define EDITOR_TREE_VISUALIZER_H


typedef struct visual_tree visual_tree;
typedef struct visual_node visual_node;

// TODO(nasr): is having 2 diff structures needed?

struct visual_tree
{       
    visual_node *parent;
    visual_node *current;
};


struct visual_node
{
    v2 pos;
    v2 size;
    str8 label;

};


#endif /* EDITOR_TREE_VISUALIZER_H */
