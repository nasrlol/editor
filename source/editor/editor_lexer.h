#ifndef EDITOR_LEXER_H
#define EDITOR_LEXER_H

typedef struct token      token;
typedef struct token_node token_node;

enum token_type
{
 TokenUndefined = 256,
 TokenIdentifier,
 TokenIdentifierValue,
 TokenString,
 TokenNumber,
 TokenDoubleEqual,
 TokenGreaterEqual,
 TokenLesserEqual,
 TokenParam,
 TokenFunc,
 TokenReturn,
 TokenIf,
 TokenElse,
 TokenFor,
 TokenWhile,
 TokenBreak,
 TokenContinue,
 TokenExpression,
 TokenWhiteSpace,
 TokenComparisonParam,
 TokenFuncBody,
 TokenUnwantedChild,
 TokenNewLine,
 TokenRightShift,
 TokenLeftShift
};

enum token_flags
{
 FlagNone            = (0),
 FlagConstant        = (1 << 0),
 FlagGlobal          = (1 << 1),
 FlagsValue          = (1 << 2),
 FlagDefinition      = (1 << 3),
 FlagComparison      = (1 << 4),
 FlagTranslationUnit = (1 << 5),
 FlagDeprecated      = (1 << 6),
 FlagDirty           = (1 << 7),
};

struct token
{
 str8        Lexeme;
 token_type  Type;
 token_flags Flags;
 u64         ByteOffset;
 s32         Column;
 s32         Line;
};

struct token_node
{
 token_node *Next;
 token_node *Previous;
 u64         ParentHandle;
 u64         ChildHandle;
 token      *Token;
};

struct token_list
{
 token_node *Root;
 token_node *Current;
};

#endif // EDITOR_LEXER_H
