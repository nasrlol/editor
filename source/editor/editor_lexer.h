#ifndef EDITOR_LEXER_H
#define EDITOR_LEXER_H
#include "base/base_core.h"
#include "base/base_strings.h"
#include "editor/editor_app.h"

typedef struct Token Token;
typedef struct SyntaxNode SyntaxNode;
typedef struct ConcreteSyntaxTree ConcreteSyntaxTree;

global_variable str8 Keywords[] =
{
    S8("break"),    S8("case"),     S8("char"),     S8("const"),
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
    '+', '-', '*', '/', '%', '=', '<', '>',
    '!', '&', '|', '^', '~',
};

/**
 *  NOTE(nasr): when is it interesting to use delimiters?
 *  this is only interesting when definging leaves and nodes of sct
 *  But when lexing we should keep in mind that a delimiter is always a token by its self
 **/
global_variable char Delimiters[] =
{
    ',', ';', '(', ')', '[',
    ']', '{', '}', '.', ':',
};

typedef enum
{
    TokenInvalid, TokenIdentifier, TokenNumber,
    TokenString, TokenChar, TokenKeyword,
    TokenOperator, TokenDelimiter, TokenPunctuation,
    TokenDirective, TokenSemicolon, TokenComment,
    TokenEOF,
} TokenType;

struct Token
{
    TokenType Type;
    str8      Lexeme;

    /**
     * NOTE(nasr): ** we should track the location of each node of the syntax tree
     * in a file so that go to definitions are possible
     * ** requires having a map of all files in the project?
     * wouldnt this take lots of space?
     * !! NO more like have something that maps the translation unit(s)
     * and works on that?
     **/
    s32       Line;
    s32       Column;

    Token    *Next;  // NOTE(nasr): linked list of tokens for efficient traversal
};

struct SyntaxNode
{
    TokenType      type;
    Token         *token;

    SyntaxNode    *parent;
    SyntaxNode   **children;
    s32            child_count;
    s32            child_capacity;
    /* NOTE(nasr): max children we can hold
     * (not sure if this something we should account for?
     * couldnt we work with checking the arena size
     * and resizing that one instead of keeping trakc of the tree size?)
     *
     * at the same time it's also something to consider because it allows us to have
     * a more defined control over the amount of nodes within a cst
     *
     * NOTE(nasr): nevermind i think it's usefull to have it because it's something
     * per node, so per branch, and if a large branch get's deleted we could do
     * a proper deletion of that entire branch which improves refactoring capabilities
     */

    /**
    * NOTE(nasr): source location tracking for go-to-definition
    * https://github.com/jqcorreia/lang/blob/main/lexer.odin
    * l54
    *
    **/
    s32            line;
    s32            column;
};

/*
 * NOTE(nasr): refactored the concrete syntax tree because the actual
 * tree struct doesnt need to know information about the actual tree
 * that information is contained within the struct
 * */
struct ConcreteSyntaxTree
{
    SyntaxNode *root;
    SyntaxNode *current;


    arena      *Arena;
};
#endif
