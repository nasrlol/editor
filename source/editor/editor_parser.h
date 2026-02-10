#ifndef EDITOR_PARSER_H
#define EDITOR_PARSER_H
#include "base/base_core.h"
#include "base/base_strings.h"
#include "editor/editor_app.h"

typedef struct Token              Token;
typedef struct SyntaxNode         SyntaxNode;
typedef struct ConcreteSyntaxTree ConcreteSyntaxTree;

/* TODO(nasr): remove these and check them inline */
// clang-format off
global_variable str8 Keywords[] =
{
S8("break"), S8("case"), S8("char"), S8("const"),
S8("continue"), S8("default"), S8("do"), S8("double"),
S8("else"), S8("enum"), S8("extern"), S8("float"),
S8("for"), S8("goto"), S8("if"), S8("int"),
S8("long"), S8("register"), S8("return"), S8("short"),
S8("signed"), S8("sizeof"), S8("static"), S8("struct"),
S8("switch"), S8("typedef"), S8("union"), S8("unsigned"),
S8("void"), S8("volatile"), S8("while"),
};

global_variable char Operators[] =
{
'+', '-', '*',
'/', '%', '=',
'<', '>', '!',
'&', '|', '^',
'~',
};

// clang-format on

typedef enum
{
    TokenInvalid = 256,
    TokenIdentifier,
    TokenNumber,
    TokenString,
    TokenChar,
    TokenKeyword,
    TokenOperator,
    TokenDelimiter,
    TokenPunctuation,
    TokenDirective,
    TokenSemicolon,
    TokenComment,
    TokenNewLine,
    TokenEOF,
} TokenType;

struct Token
{
    TokenType Type;
    str8      Lexeme;

    union
    {
        str8 StringValue;
        s32  IntegerValue;
    };

    s32 Line;
    s32 Column;

    Token *Next;
};

struct SyntaxNode
{
    TokenType Type;
    Token    *token;

    SyntaxNode  *Parent;
    SyntaxNode **Children;
    s32          ChildCount;
    s32          ChildCapacity;
};

struct ConcreteSyntaxTree
{
    SyntaxNode *root;
    SyntaxNode *current;

    arena *Arena;
};
#endif
