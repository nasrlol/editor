#ifndef EDITOR_PARSER_H
#define EDITOR_PARSER_H

typedef struct syntax_node          syntax_node;
typedef struct concrete_syntax_tree concrete_syntax_tree;
typedef struct translation_unit     translation_unit;

enum syntax_node_type
{
  SyntaxNodeLiteral,
  SyntaxNodeIdentifier,
  SyntaxNodeBinary,

  SyntaxNodeAssignment,
  SyntaxNodeReturn,
  SyntaxNodeFunction
};

struct syntax_node
{
  syntax_node *First;
  syntax_node *Last;
  syntax_node *Parent;
  syntax_node *NextNode;

  token *Token;

  syntax_node_type Type;

  union types
  {
    struct syntax_node_identifier
    {
      str8 *name;
    };

    struct syntax_node_binary
    {
      char              op;
      syntax_node_type *left;
      syntax_node_type *right;
    };

    struct syntax_node_assignment
    {
      syntax_node_type *Left;
      syntax_node_type *Right;
    };

    struct syntax_node_return_statement
    {
      syntax_node_type *expr;
    };
  };
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
.Root    = &nil_syntax_node,
.Current = &nil_syntax_node,
};

#endif // EDITOR_PARSER_H
