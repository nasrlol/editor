#ifndef EDITOR_LEXER_H
#define EDITOR_LEXER_H

typedef struct Token token;

enum TokenType
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
    TokenWhiteSpace,
};

typedef enum TokenFlags
{
	FlagNone			= 0,
	FlagConstant		= 1 << 0,
	FlagGlobal			= 1 << 1,
	FlagsValue			= 1 << 2,
	FlagDeprecated		= 1 << 3,
	FlagDefinition		= 1 << 4,
	FlagComparison		= 1 << 5,
	FlagTranslationUnit = 1 << 6,
	FlagDirty			= 1 << 7,

} TokenFlags;

struct Token
{
    str8       Lexeme;
	TokenType  Type;
	TokenFlags Flags;
	s32		   Line;
	s32		   Column;
	u64		   ByteOffset;

	/**
    * NOTE(nasr): used for the lexer part
	* since i'm trying to seperate them because of increaasing complexity
	* which i wasn't able to follow im making
    * a linked list of the current tokens and attaching
    **/
	Token *Next;
    Token *Previous;
};

struct TokenList
{
	Token *Root;
    Token *Leaf;
	s32	   Count;
};

#endif
