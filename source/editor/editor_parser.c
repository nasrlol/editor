internal ConcreteSyntaxTree *
Parse(arena *Arena, TokenList *List)
{
	ConcreteSyntaxTree *tree = PushStruct(Arena, ConcreteSyntaxTree);

	for (Token *token = List->Root; token != 0; token = token->Next)
	{
		SyntaxNode *node = PushStruct(Arena, SyntaxNode);
		node->Token		 = token;

		if (!tree->Root)
		{
			tree->Root	  = node;
			tree->Current = node;
		}

		SyntaxNode *current	 = tree->Current;
		SyntaxNode *previous = tree->Current->Parent;
		SyntaxNode *next	 = tree->Current->Parent;

		switch (token->Type)
		{
			case (TokenUndefined):
			{
				token->Flags = FlagDirty;
				// TODO(nasr): If we are ever going to implement incremental parsing this is going to be usefull
				// to do a search for the dirty flags and try fixing those up or something
				break;
			}
			case (TokenIdentifier):
			{
				if (token->Next && token->Next->Type == (TokenType)'=')
				{
					if (token->Next && token->Next->Lexeme.Size != 0)
					{
						token->Next->Next->Type = TokenIdentifierValue;
					}
				}

				break;
			}

			case (TokenIdentifierValue):
			{
				break;
			}
			case (TokenString):
			{
				if (tree->Current->Parent->Token->Type == (TokenType)'{')
				{
					tree->Current->Token;
				}

				break;
			}
			case (TokenNumber):
			{
                // TODO(nasr): handle this with strings,
                // check if its a value or part of an identifier or something
				break;
			}
			case (TokenDoubleEqual):
			{
				break;
			}
			case (TokenGreaterEqual):
			{
				break;
			}
			case (TokenLesserEqual):
			{
				break;
			}
			case (TokenRightArrow):
			{
				break;
			}
			case (TokenFunc):
			{
				break;
			}
			case (TokenReturn):
			{
				break;
			}
			case (TokenIf):
			{
				break;
			}
			case (TokenElse):
			{
				break;
			}
			case (TokenFor):
			{
                if (next && next->NextNode->Token->Type == (TokenType)'(')
                {
                    next->NextNode->NextNode->Token->Type = TokenParam;
                }

				break;
			}
			case (TokenWhile):
			{
				break;
			}
			case (TokenBreak):
			{
				break;
			}
			case (TokenContinue):
			{
				break;
			}
			case (TokenExpression):
			{
				break;
			}
		}

		tree->Current->NextNode = node;
		node->Parent			= tree->Current;
		tree->Current			= node;
	}

	return tree;
}
