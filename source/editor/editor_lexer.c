#include "base/base_core.h"
#include "base/base_os.h"
#include "base/base_strings.h"
#include "base/base_arenas.h"
#define BASE_NO_ENTRYPOINT 1
#include "editor_lexer.h"

/**
 * NOTE(nasr): what are the steps we have to follow?
 * we take in the complete file buffer?
 * but should it be the the complete file?
 * what if we have multiple files?
 *
 * what we could do is have a initial complete lexical analysis of
 * all source files, and build a concrete syntax tree, not an ast, but a concrete syntax tree
 *
 * and on a file save or change try to update the concrete syntax tree
 *  but it would be nice if the concrete syntax tree could change without needing to have to
 *  save the file after each edit or change
 *
 *  but doing that would mean that we need to have a LSP server that does a continous analysis
 *
 **/

/**
 * NOTE(nasr): what im trying to do here is do a regular parsing scan over the
 * complete buffer, seperate words on spaces and other general delimters
 * and get a list of tokens out of this so i can run the tokenizer of a list
 * */

/**
 * NOTE(nasr): new idea, make a parse -> lexing pipeline
 * we make continous parsing for the continous input
 * and then we do a search on the nodes of the cst
 **/

internal Token *ParseBuffer(app_state *app, arena *Arena)
{

    /*
     * Lets say we have a line \\ int main() { return 0; }
     * this line shows that a function has a return type, a name, braces
     * and the scope of the function starts end ends with braces { }
     * so the tree should shows
     * [function(main(return type)(name)(parameters))[scope[and here have more stuff]]]
     * now the tricky part about c is that most things are seperated by spaces but not all
     * T value = {0}; \\ this for example is allowed.
     **/

    /*
     * TODO(nasr): think about the new lines,
     * a new line doesnt define a new branch or scope of syntax
     * but it is often used?
     *
     * do we handle it or not?
     * */
    /**
     * I think i misunderstood the difference between a parser and a lexer
     **/

    Token *FirstToken = 0;
    Token *LastToken = 0;

    s32 TextIndex = 0;
    s32 Line = 1;
    s32 Column = 1;

    while(TextIndex < app->TextCount)
    {
        rune CurrentChar = app->Text[TextIndex];

        // NOTE(nasr): skip whitespace but track position
#if !defined(OS_WINDOWS)
        while(TextIndex < app->TextCount &&
              (CurrentChar == ' ' || CurrentChar == '\t' || CurrentChar == '\n' || CurrentChar == '\r' /*windows support*/))
#endif
#if defined(__linux__)
        while(TextIndex < app->TextCount &&
              (CurrentChar == ' ' || CurrentChar == '\t' || CurrentChar == '\n'))
#endif
        {
            if(CurrentChar == '\n')
            {
                ++Line;
                Column = 1;
            }
            else
            {
                ++Column;
            }
            TextIndex += 1;
            if(TextIndex < app->TextCount) { CurrentChar = app->Text[TextIndex]; }
        }

        if(TextIndex >= app->TextCount)
        {
            break;
        }

        s32 TokenStart = TextIndex;
        s32 TokenStartColumn = Column;
        b32 IsDelimiter = false;

        // NOTE(nasr): check if current char is a delimiter
        for(s32 DelimiterIndex = 0; DelimiterIndex < ArrayCount(Delimiters); ++DelimiterIndex)
        {
            if(Delimiters[DelimiterIndex] == (char)CurrentChar)
            {
                IsDelimiter = true;
                break;
            }
        }

        s32 TokenEnd = TextIndex;

        if(IsDelimiter)
        {
            TokenEnd = TextIndex + 1;
            Column += 1;
        }
        else
        {
            // NOTE(nasr): scan until we hit whitespace or delimiter
            while(TextIndex < app->TextCount)
            {
                CurrentChar = app->Text[TextIndex];

                if(CurrentChar == ' ' || CurrentChar == '\t' || CurrentChar == '\n' || CurrentChar == '\r')
                {
                    break;
                }

                b32 IsDelimiter = false;
                for(s32 DelimiterIndex = 0; DelimiterIndex < ArrayCount(Delimiters); ++DelimiterIndex)
                {
                    if(Delimiters[DelimiterIndex] == (char)CurrentChar)
                    {
                        IsDelimiter = true;
                        break;
                    }
                }

                if(IsDelimiter)
                {
                    break;
                }

                TextIndex += 1;
                Column += 1;
            }

            TokenEnd = TextIndex;
        }

        s32 TokenSize = TokenEnd - TokenStart;
        if(TokenSize > 0)
        {
            Token *NewToken = PushStruct(Arena, Token);

            u8 *TokenStr = PushArray(Arena, u8, TokenSize + 1);
            for(s32 i = 0; i < TokenSize; ++i)
            {
                TokenStr[i] = (u8)app->Text[TokenStart + i];
            }
            TokenStr[TokenSize] = 0;

            NewToken->Lexeme.Data = TokenStr;
            NewToken->Lexeme.Size = TokenSize;
            NewToken->Line = Line;
            NewToken->Column = TokenStartColumn;
            NewToken->Type = TokenInvalid;
            NewToken->Next = 0;

            if(LastToken)
            {
                LastToken->Next = NewToken;
                LastToken = NewToken;
            }
            else
            {
                FirstToken = NewToken;
                LastToken = NewToken;
            }
        }

        TextIndex = TokenEnd;
    }


    // NOTE(nasr): check for keywords
    for(umm KeywordIndex = 0; KeywordIndex < ArrayCount(Keywords); ++KeywordIndex)
    {
        if(S8Match(Token->Lexeme, Keywords[KeywordIndex], false))
        {
            TokenOut->Type = TokenKeyword;
            return TokenOut;
        }
    }

    // NOTE(nasr): check for single-character tokens
    if(ParsedWord.Size == 1)
    {
        char c = (char)ParsedWord.Data[0];

        /*
         * TODO(nasr): how do we handle unary expressions?
         * */
        if(c == '+' || c == '-' || c == '*' || c == '/')
        {
            TokenOut->Type = TokenOperator;
            return TokenOut;
        }

        /*
         * for preprocessor stuff
         * #include #endif
         * */
        if(c == '#')
        {
            TokenOut->Type = TokenDirective;
            return TokenOut;
        }

        if(c == ';')
        {
            TokenOut->Type = TokenSemicolon;
            return TokenOut;
        }

        if(c == '(' || c == ')' || c == '{' || c == '}' || c == '[' || c == ']')
        {
            TokenOut->Type = TokenDelimiter;
            return TokenOut;
        }

        if(c == ',' || c == '.')
        {
            TokenOut->Type = TokenPunctuation;
            return TokenOut;
        }
    }

    // NOTE(nasr): check if it's a number literal
    if(ParsedWord.Size > 0)
    {
        char FirstChar = (char)ParsedWord.Data[0];
        if(FirstChar >= '0' && FirstChar <= '9')
        {
            TokenOut->Type = TokenNumber;
            return TokenOut;
        }
    }

    // NOTE(nasr): check if it's a string literal
    if(ParsedWord.Size >= 2)
    {
        char FirstChar = (char)ParsedWord.Data[0];
        char LastChar = (char)ParsedWord.Data[ParsedWord.Size - 1];
        if((FirstChar == '"' && LastChar == '"') || (FirstChar == '\'' && LastChar == '\''))
        {
            TokenOut->Type = TokenString;
            return TokenOut;
        }
    }

    // NOTE(nasr): if nothing else, it's probably an identifier
    if(ParsedWord.Size > 0)
    {
        /* TODO(nasr): what are the possible identifier starters? */
        char FirstChar = (char)ParsedWord.Data[0];
        if((FirstChar >= 'a' && FirstChar <= 'z') ||
           (FirstChar >= 'A' && FirstChar <= 'Z'))
        {
            TokenOut->Type = TokenIdentifier;
            return TokenOut;
        }
    }

    return TokenOut;
}

/**
 * NOTE(nasr): The tokenize function takes the parsed buffer and tokenizes
 * every element in the list
 * We try to identify the leximes, that were previously split up;
 * */


/*
 * NOTE(nasr):
 * lets say we have a set of tokens
 * [[int][main][(][int][argc]][)][{][}]]
 * how do we build a tree out of this?
 * */
internal void AddToCST(ConcreteSyntaxTree *tree, SyntaxNode *node)
{

        /* NOTE(nasr): building the concretee syntax tree */

        /**
         * What defines the root of a concrete syntax tree
         * when do branches get created?
         * not at the start of a line because this is possible ->
         * \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
         * internal int
         * main(int argc, char **argv)
         * { retur 0; }
         * \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
         **/

        /* NOTE(nasr): we know that a semicolon defines the end of of a branch */

    /**
     * check if a token is a sort of token
     * does that token take children or something else
     **/
    if(tree->root == 0)
    {
        tree->root = node;
        tree->current = node;
    }

    else
    {
        if(tree->current->child_count < tree->current->child_capacity)
        {
            tree->current->children[tree->current->child_count] = node;
            tree->current->child_count += 1;
            node->parent = tree->current;
        }
        else
        {
            Log("max tree");

            /* TODO(nasr): expand tree or something */
        }
    }
}


internal void RemoveFromCST(ConcreteSyntaxTree *tree, SyntaxNode node)
{
    /**
     * TODO(nasr):
     * 1. find the node
     * 2. remove the node
     * 3. restructure the thing
     * **/
}
