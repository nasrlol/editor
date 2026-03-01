#ifndef EDITOR_TREE_VISUALIZER_H
#define EDITOR_TREE_VISUALIZER_H

typedef struct visual_node visual_node;
struct visual_node
{
    v2           Pos;
    v2           Size;
    str8         Label;
    v4           Color;
    visual_node *Parent;
    visual_node *Next;
    visual_node *First;
    visual_node *Last;
};

typedef struct visual_tree visual_tree;
struct visual_tree
{
    visual_node *Root;
    s32          Count;
};

#endif // EDITOR_TREE_VISUALIZER_H
