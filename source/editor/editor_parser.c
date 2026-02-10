#include "base/base_core.h"
#include "base/base_os.h"
#include "base/base_strings.h"
#include "base/base_arenas.h"
#define BASE_NO_ENTRYPOINT 1
#include "editor_parser.h"

internal Token *
Lex(app_state *app, arena *Arena)
{
    Token *FirstToken = PushStruct(Arena, Token);
    Token *LastToken  = PushStruct(Arena, Token);

    s32 TextIndex = 0;
    s32 Line      = 1;
    s32 Column    = 1;

    while (TextIndex < app->TextCount)
    {
        rune c = app->Text[TextIndex];

        while ((c == ' ' || c == '\t' || c == '\n' || c == '\r'))
        {
            if (c == '\n')
            {
                ++Line;
                Column = 1;
            }
            else
            {
                ++Column;
            }

            TextIndex += 1;
            if (TextIndex <= app->TextCount)
            {
                c = app->Text[TextIndex];
            }
        }

        if (TextIndex >= app->TextCount)
        {
            break;
        }

        s32 TokenStart       = TextIndex;
        s32 TokenEnd         = TextIndex;
        s32 TokenStartColumn = Column;
        s32 TokenSize        = 0;

        c = app->Text[TextIndex];

        if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
        {
            continue;
        }

        if (TokenEnd > TokenStart)
        {
            TokenSize = TokenEnd - TokenStart;
        }

        if (TokenSize > 0)
        {
            Token *NewToken = PushStruct(Arena, Token);
            u8    *TokenStr = PushArray(Arena, u8, TokenSize + 1);

            for (s32 TokenSizeIndex = 0;
            TokenSizeIndex < TokenSize;
            ++TokenSizeIndex)
            {
                TokenStr[TokenSizeIndex] = (u8)app->Text[TokenStart + TokenSizeIndex];
            }

            TokenStr[TokenSize] = 0;

            NewToken->Lexeme.Data = TokenStr;
            NewToken->Lexeme.Size = TokenSize;
            NewToken->Line        = Line;
            NewToken->Column      = TokenStartColumn;

            if (NewToken->Lexeme.Size == 1)
            {
                char c = (char)NewToken->Lexeme.Data[0];

                if (c == '+' || c == '-' || c == '*' || c == '/')
                {
                    NewToken->Type = TokenOperator;
                }

                if (c == '#')
                {
                    NewToken->Type = TokenDirective;
                }

                if (c == ';')
                {
                    NewToken->Type = TokenSemicolon;
                }

                {
                    s32 OpeningDelimiter, ClosingDelimiter = 0;

                    if (c == '(' || c == '{' || c == '[')
                    {
                        OpeningDelimiter = TextIndex;
                    }

                    if (c == ')' || c == '}' || c == ']')
                    {
                        ClosingDelimiter = TextIndex;
                    }

                    if (ClosingDelimiter >= OpeningDelimiter)
                    {
                        break;
                    }
                    else
                    {
                        // TODO(nasr): save value
                    }

                    TokenEnd = TextIndex + 1;
                    Column += 1;
                    NewToken->Type = TokenDelimiter;
                }

                if (c == ',' || c == '.')
                {
                    NewToken->Type = TokenPunctuation;
                }
            }

            if (NewToken->Lexeme.Size >= 2)
            {
                char FirstChar = (char)NewToken->Lexeme.Data[0];
                char LastChar  = (char)NewToken->Lexeme.Data[NewToken->Lexeme.Size - 1];
                if ((FirstChar == '"' && LastChar == '"') ||
                    (FirstChar == '\'' && LastChar == '\''))
                {
                    NewToken->Type = TokenString;
                }
            }

            if (NewToken->Lexeme.Size > 0)
            {
                char FirstChar = (char)NewToken->Lexeme.Data[0];
                if ((FirstChar >= 'a' && FirstChar <= 'z') ||
                    (FirstChar >= 'A' && FirstChar <= 'Z') ||
                    (FirstChar >= '0' && FirstChar <= '9'))
                {
                    NewToken->Type = TokenIdentifier;
                }
            }

            NewToken->Next = PushStruct(Arena, Token);

            if (LastToken)
            {
                LastToken->Next = NewToken;
                LastToken       = NewToken;
            }
            else
            {
                FirstToken = NewToken;
                LastToken  = NewToken;
            }

            TextIndex = TokenEnd;
        }

        ++TextIndex;
    }

    return FirstToken;
}

internal void
AddToCST(ConcreteSyntaxTree *tree, SyntaxNode *node)
{
    if (tree->root == 0)
    {
        tree->root    = node;
        tree->current = node;
    }

    else
    {
        if (tree->current->ChildCount < tree->current->ChildCapacity)
        {
            tree->current->Children[tree->current->ChildCount] = node;
            tree->current->ChildCount += 1;
            node->Parent = tree->current;
        }
        else
        {
            Log("max tree");

            /* TODO(nasr): (see notess)*/
        }
    }
}

internal void
RemoveFromCST(ConcreteSyntaxTree *tree, SyntaxNode node)
{
    /*TODO(nasr): (see notes) */
}
