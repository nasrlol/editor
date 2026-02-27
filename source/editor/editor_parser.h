#ifndef EDITOR_PARSER_H
#define EDITOR_PARSER_H

typedef struct syntax_node          syntax_node;
typedef struct concrete_syntax_tree concrete_syntax_tree;
typedef struct translation_unit     translation_unit;

struct syntax_node
{
    syntax_node *First;
    syntax_node *Last;
    syntax_node *Parent;
    syntax_node *NextNode;
    token       *Token;

};

struct concrete_syntax_tree
{
    syntax_node *Root;
    syntax_node *Current;
};

// TODO(nasr): implement this later together with file handling
struct translation_unit
{
    s32                   FileID;
    concrete_syntax_tree *Tree;
};

read_only global_variable
syntax_node nil_syntax_node =
{
    .First    = &nil_syntax_node,
    .Last     = &nil_syntax_node,
    .Parent   = &nil_syntax_node,
    .NextNode = &nil_syntax_node,
    .Token    = &nil_token,
    };

read_only global_variable
concrete_syntax_tree nil_concrete_syntax_tree =
{
    .Root = &nil_syntax_node,
    .Current = &nil_syntax_node,
};


#endif // EDITOR_PARSER_H
