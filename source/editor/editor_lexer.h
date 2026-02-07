#ifndef EDITOR_LEXER_H
#define EDITOR_LEXER_H

#include "base/base_core.h"
#include "base/base_strings.h"

typedef struct Token Token;
typedef struct SyntaxNode SyntaxNode;
typedef struct ConcreteSyntaxTree ConcreteSyntaxTree;

global_variable str8 Keywords[] =
{
    S8("break"),    S8("case"),     S8("char"), S8("const"),
    S8("continue"), S8("default"),  S8("do"),
    S8("double"),   S8("else"),     S8("enum"),     S8("extern"),
    S8("float"),    S8("for"),      S8("goto"),     S8("if"),
    S8("int"),      S8("long"),     S8("register"), S8("return"),
    S8("short"),    S8("signed"),   S8("sizeof"),   S8("static"),
    S8("struct"),   S8("switch"),   S8("typedef"),  S8("union"),
    S8("unsigned"), S8("void"),     S8("volatile"), S8("while"),

};

global_variable char Operators[] =
{
    '+',
    '-',
    '*',
    '/',
    '%',
    '=',
    '<',
    '>',
};

/**
 *  NOTE(nasr): when is it interesting to use delimiters?
 *  this is only interesting when definging leaves and nodes of sct
 *  But when lexing we should keep in mind that a delimiter is always a token by its self
 **/
global_variable char Delimiters[] =
{
    ' ',
    ',',
    ';',
    '(',
    ')',
    '[',
    ']',
    '{',
    '}',
};

typedef enum
{
    Token_Invalid, Token_Identifier, /*more like a value? does this get sperated in  a different thing?*/ Token_Number, Token_String,
    Token_Char, Token_Keyword, Token_Operator, Token_Punctuator,
    Token_Directive, Token_Whitespace, Token_Newline, Token_Comment,
    Token_Semicolon

} TokenType;


struct Token
{
    TokenType Type;
    str8      Lexeme;
    /**
     * NOTE(nasr):
     * ** we should track the location of each node of the syntax tree in a file so that go to definitions are possible
     * ** requires having a map of all files in the project? wouldnt this take lots of space?
     *
     * !! NO more like have something that maps the translation unit(s) and works on that?
     **/
    s32       Line;
    s32       Column;
};

struct ConcreteSyntaxTree
{
    union
    {
        struct {
            SyntaxNode *leftNode;
            SyntaxNode *rightNode;
        };

        struct
        {
            SyntaxNode *node;
        };
    };
};

struct SyntaxNode
{
    TokenType type;
    union
    {
        struct
        {
            SyntaxNode **node;
            int child_count;
        };

        Token token;
    };
};


#endif


