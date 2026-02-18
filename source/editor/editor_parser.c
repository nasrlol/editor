internal ConcreteSyntaxTree *
Parse(app_state *app, arena *Arena, b32 FormatEverything)
{
    ConcreteSyntaxTree *tree = PushStruct(Arena, ConcreteSyntaxTree);

    s32 TextIndex = 0;
    s32 Line      = 1;
    s32 Column    = 1;

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
            u8    *StrValue= PushArray(Arena, u8, TokenSize + 1);

            for (s32 i = 0; i < TokenSize; ++i)
            {
                StrValue[i] = (u8)app->Text[TokenStart + i];
            }
            StrValue[TokenSize] = 0;

            NewToken->Lexeme.Data = StrValue;
            NewToken->Lexeme.Size = TokenSize;
            NewToken->Line        = Line;
            NewToken->Column      = Column;

            if (TokenSize == 1)
            {
                char ch = (char)StrValue[0];

                if (ch >= '0' && ch <= '9')
                {
                    NewToken->Type = (TokenType)'+';
                    break;
                }

                switch (ch)
                {
                    case ('+'):
                    {
                        NewToken->Type = (TokenType)'+';
                        break;
                    }

                    case ('-'):
                    {
                        NewToken->Type = (TokenType)'-';
                        break;
                    }

                    case ('*'):
                    {
                        NewToken->Type = (TokenType)'*';
                        break;
                    }
                    case ('/'):
                    {
                        NewToken->Type = (TokenType)'*';
                        break;
                    }
                    case (';'):
                    {
                        NewToken->Type = (TokenType)'*';
                        break;
                    }
                    case (','):
                    {
                        NewToken->Type = (TokenType)'*';
                        break;
                    }
                    case ('.'):

                    {
                        NewToken->Type = (TokenType)'*';
                        break;
                    }
                    case ('('):
                    {
                        NewToken->Type = (TokenType)'*';
                        break;
                    }
                    case ('{'):
                    {
                        NewToken->Type = (TokenType)'*';
                        break;
                    }
                    case ('['):
                    {
                        NewToken->Type = (TokenType)'*';
                        break;
                    }
                    case (')'):
                    {
                        NewToken->Type = (TokenType)'*';
                        break;
                    }
                    case ('}'):
                    {
                        NewToken->Type = (TokenType)'*';
                        break;
                    }
                    case (']'):
                    {
                        NewToken->Type = (TokenType)'*';
                        break;
                    }
                    case ('<'):
                    {
                        if (StrValue[1] == '=')
                        {
                            NewToken->Type = TokenGreaterEqual;
                        }
                        else
                        {
                            NewToken->Type = (TokenType)'<';
                        }
                        break;
                    }
                }
            }

            else if (TokenSize >= 2)
            {
                char FirstChar = (char)StrValue[0];
                char LastChar  = (char)StrValue[TokenSize - 1];

                if ((FirstChar == '"' && LastChar == '"') ||
                    (FirstChar == '\'' && LastChar == '\''))
                {
                    NewToken->Type = TokenString;
                }

                for (s32 TokenStrIndex = 0;
                TokenStrIndex < TokenSize;
                ++TokenStrIndex)
                {
                    /* TODO(nasr): handle keywords*/
                }
            }

            Column += TokenSize;
            {
                SyntaxNode *node = PushStruct(Arena, SyntaxNode);
                node->Token      = NewToken;

                if (tree->Root == 0)
                {
                    tree->Root    = node;
                    tree->Current = node;
                }
                else
                {
                    node->NextNode = node;
                }

                switch (node->Token->Type)
                {
                    case TokenUndefined:
                    {
                        ErrorLog("bad Token");
                        break;
                    }
                    case TokenIdentifier:
                    {
                        /*
                             *  1. peek
                             *  2. equals something
                             *  3. restructure
                             *
                             *  (int) -> (x)-> (=)-> (5)-> (+) -> (3) -> (;)
                             * */

                        if (node->NextNode->Token->Type == (TokenType)'=')
                        {
                            node->Child[0] = node->NextNode;
                            node->Child[1] = node->NextNode->NextNode;
                        }
                        break;
                    }

                    case TokenIdentifierName:
                    {
                        if (node->Parent->Token->Type == TokenIdentifier)
                        {
                            node->Token->Type = TokenIdentifier;
                        }
                    }
                    case TokenNumber:
                    case TokenString:
                    {
                        if (node->Parent->Token->Type == TokenIdentifier)
                        {
                            node->Parent->Child = &node;
                        }
                    }

                    case TokenDoubleEqual:
                    {
                        node->Token->Flags |= FlagDefinition;
                        break;
                    }

                    case TokenLesserEqual:
                    case TokenGreaterEqual:
                    {
                        if (node->NextNode->Token->Type == TokenString ||
                            node->NextNode->Token->Type == TokenNumber)
                        {
                            // assign the value
                            if (node->NextNode)
                            {
                                node->Child[0] = node->NextNode;
                            }
                            else
                            {
                                Log("Incomplete operation");
                            }

                            if (node->NextNode->Token->Type == TokenNumber)
                            {
                                // TODO(nasr): we could do some cool things here
                            }
                        }
                    }

                    case TokenRightArrow:
                    {
                        // TODO(nasr): handle types, including self defined types/structs
                        break;
                    }
                    case TokenFunc:
                    {
                        /**
                        * TODO(nasr): think about how to handle functions
                        * what defines a function?
                        */
                        break;
                    }
                    case TokenReturn:
                    {
                        if (node->NextNode &&
                            node->NextNode->Token->Type == TokenIdentifier)
                        {
                            node->Child[0] = node->NextNode;
                        }
                    }
                    case TokenIf:
                    case TokenElse:
                    {
                        /**
                        * TODO(nasr): what is the relation between else and if?
                        * is it a Next Relation or Child Relation
                        **/
                    }

                    case (TokenBreak):
                    case (TokenContinue):
                    {
                        SyntaxNode *CurrentNode = PushStruct(Arena, SyntaxNode);
                        CurrentNode             = node;

                        while (node->Parent->Token->Type != TokenFor ||
                               node->Parent->Token->Type != TokenIf ||
                               node->Parent->Token->Type != TokenWhile)
                        {
                            CurrentNode = node->Parent;
                        }
                    }
                    case (TokenFor):
                    {
                        if (node)
                        {

                        }
                        break;
                    }
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
