#ifndef EDITOR_PARSER_H
#define EDITOR_PARSER_H

typedef struct SyntaxNode		  SyntaxNode;
typedef struct ConcreteSyntaxTree ConcreteSyntaxTree;
typedef struct TranslationUnit	  TranslationUnit;

struct SyntaxNode
{
	SyntaxNode	*Parent;
	SyntaxNode **Child;

	SyntaxNode *NextNode;
	token	   *Token;

	umm Scope;
};

struct ConcreteSyntaxTree
{
	SyntaxNode *Root;
	SyntaxNode *Current;
};

struct TranslationUnit
{
	s32					FileID;
	ConcreteSyntaxTree *Tree;
};

#endif
