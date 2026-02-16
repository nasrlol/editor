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

		switch ((int)token->Type)
		{
			case (TokenUndefined):
			{
                tree->Current->Token->Flags = (TokenFlags)(tree->Current->Token->Flags | FlagDirty);
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

			case ((TokenType)'{'):
			{
				if (tree->Current->Parent->Token->Type == TokenFunc)
				{
                    SyntaxNode *node = PushStruct(Arena, SyntaxNode);
                    while (node->NextNode)
                    {

                    }
					// TODO(nasr):
				}
			}

			case (TokenIdentifierValue):
			{
                // TODO(nasr): check if the previous nodes are correct etc;

				break;
			}

			case (TokenNumber):
			case (TokenString):
			{
				if (tree->Current->Parent->Token->Type == (TokenType)'{')
				{
                    // TODO(nasr): i was doing something with this but forgot what :)
					// tree->Current->Token;
				}
				else if (tree->Current->Parent->Token->Type == (TokenType)'=')
				{
					tree->Current->Token->Type = TokenIdentifierValue;

					if (tree->Current->Parent->Token->Type == TokenIdentifier)
					{
						// TODO(nasr): macro for simplifying this
						tree->Current->Token->Flags = (TokenFlags)(tree->Current->Token->Flags | FlagDirty);
					}
				}

				break;
			}

			case (TokenDoubleEqual):
			case (TokenGreaterEqual):
			case (TokenLesserEqual):
			{
				break;
			}

			case ((TokenType)'<'):
			{
				break;
			}

			case ((TokenType)'>'):
			{
				break;
			}

			case (TokenFunc):
			{
				if (tree->Current->NextNode->Token->Type == (TokenType)'(')
				{
					SyntaxNode *BodyNode = PushStruct(Arena, SyntaxNode);
					// find closing bracket
					SyntaxNode *nextNode = tree->Current->NextNode;

					while (nextNode)
					{
						if ((nextNode->Token->Type != (TokenType)'}'))
						{
						}
						else
						{
							// TODO(nasr): macro for simplifying this
							tree->Current->Token->Flags = (TokenFlags)(tree->Current->Token->Flags | FlagDirty);
							break;
						}

						if (tree->Current->NextNode)
						{
							break;
						}

						nextNode = tree->Current->NextNode;
					}

                    /**
                     * NOTE(nasr): first child is opening bracket
                     * second child is body
                     * third one is closing bracket 
                     **/

					tree->Current->Child[0]->Token->Type = (TokenType)'{';
					tree->Current->Child[1]->Token->Type = TokenFuncBody;
					tree->Current->Child[2]->Token->Type = (TokenType)'{';
				}
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
                if (tree->Current->NextNode->Token->Type == (TokenType)'{')
                {

                }

				break;
			}

			case (TokenWhile):
			case (TokenFor):
			{
				SyntaxNode *BodyNode = PushStruct(Arena, SyntaxNode);
				// find closing bracket
				SyntaxNode *nextNode = tree->Current->NextNode;
				if (nextNode && nextNode->NextNode->Token->Type == (TokenType)'(')
				{
					nextNode->NextNode->NextNode->Token->Type = TokenParam;
				}

				break;
			}

			case (TokenBreak):
			{
                /**
                * this assumes the parent is correct
                **/
                if (!tree->Current->Parent)
                {    
                    tree->Current->Token->Flags = (TokenFlags)(tree->Current->Token->Flags | FlagDirty);
                }
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
