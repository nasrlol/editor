#define BASE_NO_ENTRYPOINT 1
#include "base/base_core.h"
#include "editor_parser.h"

internal ConcreteSyntaxTree *
Parse(app_state *app, arena *Arena, b32 FormatEverything)
{
    ConcreteSyntaxTree *tree;
    s32                 TextIndex = 0;
    s32                 Line      = 1;
    s32                 Column    = 1;

    while (TextIndex < app->TextCount)
    {
        rune c = app->Text[TextIndex];

        if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
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
            ++TextIndex;
            continue;
        }

        s32 TokenStart = TextIndex;
        s32 TokenEnd   = TextIndex;

        while (TokenEnd < app->TextCount)
        {
            rune ch = app->Text[TokenEnd];
            if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r')
            {
                break;
            }
            ++TokenEnd;
        }

        s32 TokenSize = TokenEnd - TokenStart;

        if (TokenSize > 0)
        {
            Token *NewToken = PushStruct(Arena, Token);
            u8    *TokenStr = PushArray(Arena, u8, TokenSize + 1);

            for (s32 i = 0; i < TokenSize; ++i)
            {
                TokenStr[i] = (u8)app->Text[TokenStart + i];
            }
            TokenStr[TokenSize] = 0;

            NewToken->Lexeme.Data = TokenStr;
            NewToken->Lexeme.Size = TokenSize;
            NewToken->Line        = Line;
            NewToken->Column      = Column;
            NewToken->Type        = TokenIdentifier;
            NewToken->Next        = 0;

            if (TokenSize == 1)
            {
                char ch = (char)TokenStr[0];
                switch (ch)
                {
                    case '+':
                    case '-':
                    case '*':
                    case '/':
                        NewToken->Type = TokenOperator;
                        break;

                    case '#':
                        NewToken->Type = TokenDirective;
                        break;

                    case ';':
                        NewToken->Type = TokenSemicolon;
                        break;

                    case ',':
                    case '.':
                        NewToken->Type = TokenPunctuation;
                        break;

                    case '(':
                    case '{':
                    case '[':
                    case ')':
                    case '}':
                    case ']':
                        NewToken->Type = TokenDelimiter;
                        break;
                }
            }

            else if (TokenSize >= 2)
            {
                char FirstChar = (char)TokenStr[0];
                char LastChar  = (char)TokenStr[TokenSize - 1];

                if ((FirstChar == '"' && LastChar == '"') ||
                    (FirstChar == '\'' && LastChar == '\''))
                {
                    NewToken->Type = TokenString;
                }
            }

            Column += TokenSize;

            SyntaxNode *node = PushStruct(Arena, SyntaxNode);
            node->token      = NewToken;

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
                }
            }
        }

        TextIndex = TokenEnd;
    }

    return tree;
}

internal void
RemoveFromCST(ConcreteSyntaxTree *tree, SyntaxNode node)
{
    /*TODO(nasr): (see notes) */
}
