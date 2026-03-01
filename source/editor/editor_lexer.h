   #ifndef EDITOR_LEXER_H
#define EDITOR_LEXER_H

enum token_type
{
    TokenUndefined = 256,
    TokenIdentifier,
    TokenIdentifierAssignmentValue,
    TokenValue,
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
    TokenLeftShift,
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

typedef struct token token;
struct token
{
    str8        Lexeme;
    token_type  Type;
    token_flags Flags;
    u64         ByteOffset;
    s32         Column;
    s32         Line;
};

typedef struct token_node token_node;
struct token_node
{
    token_node *Next;
    token_node *Previous;
    u64         ParentHandle;
    u64         ChildHandle;
    token      *Token;
};

typedef struct token_list token_list;
struct token_list
{
    token_node *Root;
    token_node *Current;
};

typedef struct lexer lexer;
struct lexer
{
    u8 *Text;
    u64 TextCount;
    u8 *EndOfFile;
    u8 *UndefinedTokens;
};

global_variable const rune Delimiters[] =
{
    '{', '}', '(', ')', '[', ']', ';',
};

read_only global_variable token nil_token =
{
    .Lexeme     = {NULL, 0},
    .Type       = TokenUndefined,
    .Flags      = FlagNone,
    .ByteOffset = 0,
    .Column     = 0,
    .Line       = 0,
};

read_only global_variable token_node nil_token_node =
{
    .Next         = &nil_token_node,
    .Previous     = &nil_token_node,
    .ParentHandle = 0,
    .ChildHandle  = 0,
    .Token        = NULL,
};

#endif // EDITOR_LEXER_H
