#ifndef EDITOR_PARSER_H
#define EDITOR_PARSER_H

typedef struct Token              token;
typedef struct SyntaxNode         SyntaxNode;
typedef struct ConcreteSyntaxTree ConcreteSyntaxTree;
typedef struct TranslationUnit    TranslationUnit;

enum TokenType
{
    TokenUndefined = 256,
    TokenIdentifier,
    TokenIdentifierName,
    TokenIdentifierValue,
    TokenString,
    TokenNumber,
    TokenDoubleEqual,
    TokenGreaterEqual,
    TokenLesserEqual,
    TokenRightArrow,
    TokenFunc,
    TokenReturn,
    TokenIf,
    TokenElse,
    TokenFor,
    TokenWhile,
    TokenBreak,
    TokenContinue,
    TokenExpression,
};

typedef enum TokenFlags
{
    FlagNone            = 0,
    FlagConstant        = 1 << 0,
    FlagGlobal          = 1 << 1,
    FlagsValue          = 1 << 2,
    FlagDeprecated      = 1 << 3,
    FlagDefinition      = 1 << 4,
    FlagComparison      = 1 << 5,
    FlagTranslationUnit = 1 << 6,
    FlagDirty           = 1 << 7,

} TokenFlags;

struct Token
{
    str8       Lexeme;
    TokenType  Type;
    TokenFlags Flags;
    s32        Line;
    s32        Column;
};

struct SyntaxNode
{
    SyntaxNode  *Parent;
    SyntaxNode **Child;

    SyntaxNode *NextNode;
    token      *Token;

    umm Scope;
};

struct ConcreteSyntaxTree
{
    SyntaxNode *Root;
    SyntaxNode *Current;

    arena *Arena;
};

struct TranslationUnit
{
    s32                 FileID;
    ConcreteSyntaxTree *Tree;
};

#endif
