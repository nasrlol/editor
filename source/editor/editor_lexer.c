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
internal str8 *ParseBuffer(app_state *app)
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

    str8 *tokens = {0};

    s32 start = -1;
    s32 end   = 0;

    for (s32 TextIndex = 0;
            TextIndex < app->TextCount;
            ++TextIndex)
    {
        char CurrentChar = app->Text[TextIndex];
        {

            if ((( CurrentChar != ' ' || CurrentChar == '\t' /* || CurrentChar  == '\n' */ ))  && start < 0)
            {
                start = TextIndex;
            }
            else if ((( CurrentChar == ' ' ||
                            CurrentChar  == '\t' ||
                            TextIndex == app->TextCount - 1))
                    && start >= 0)
            {
                if (CurrentChar == ' ' || CurrentChar == '\t')
                {
                    end = TextIndex;
                }
                else
                {
                    end = TextIndex + 1;
                }
                break;
            }

            else
            {
                for (s32 DelimiterIndex = 0;
                        DelimiterIndex < ArrayCount(Delimiters);
                        ++DelimiterIndex)
                {
                    if (Delimiters[DelimiterIndex] == CurrentChar)
                    {
                        start = DelimiterIndex;

                    }
                }
            }

            if (start < 0)
            {
                *tokens = (str8){0};
            }

        }

                *tokens = S8FromTo(S8(app->Text) , start, end);
                ++tokens;
    }

    return tokens;
}
/**
 * NOTE(nasr): The tokenize function takes the parsed buffer and tokenizes
 * every element in the list
 * We try to identify the leximes, that were previously split up;
 * */





internal Token *Tokenize(str8 ParsedWord)
{

    /* NOTE(nasr):
     * maybe put the tokenizing in a seperate function
     * no need for doing that at the moment */

    Token TokenOut = {sizeof(Token)};

    TokenOut.Type   = Token_Invalid;
    TokenOut.Lexeme = ParsedWord;
    TokenOut.Line   = 0;
    TokenOut.Column = 0;

    for (umm KeywordIndex = 0;
            KeywordIndex < ArrayCount(Keywords);
            ++KeywordIndex)
    {
        if (S8Match(ParsedWord, Keywords[KeywordIndex], false))
        {
            TokenOut.Type = Token_Keyword;
            break;
        }
    }

    /*
     * TODO(nasr): how do we handle unary expressions?
     * */
    if (ParsedWord.Data == (u8 *)'+' || ParsedWord.Data == (u8 *)'-' ||
            ParsedWord.Data == (u8 *)'*' || ParsedWord.Data == (u8 *) '/')
    {
        TokenOut.Type = Token_Operator;
    }


    /*
     * for preprocessor stuff
     * #include #endif
     * */
    if (ParsedWord.Data == (u8 *)'#' )
    {
        TokenOut.Type = Token_Directive;
    }

    if (ParsedWord.Data == (u8 *)';' )
    {
        TokenOut.Type = Token_Semicolon;
    }
    return &TokenOut;
}


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
     *
     * */
        ConcreteSyntaxTree

    if (node->type == Function)
    if (node->child_count == 0)
    {
        tree->node = node;
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
