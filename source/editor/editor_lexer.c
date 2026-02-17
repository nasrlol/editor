internal TokenList *
Lex(app_state *app, arena *Arena)
{
	TokenList *List		 = PushStruct(Arena, TokenList);
	s32		   TextIndex = 0;
	s32		   Line		 = 1;
	s32		   Column	 = 1;

	while (TextIndex < app->TextCount)
	{
		rune character = app->Text[TextIndex];

		if (character == ' ' || character == '\t')
		{
			TextIndex++;
			Column++;
			continue;
		}
		if (character == '\n' || character == '\r')
		{
			TextIndex++;
			Line++;
			Column = 1;
			continue;
		}

		Token *NewToken	 = PushStruct(Arena, Token);
		NewToken->Line	 = Line;
		NewToken->Column = Column;

		s32 TokenStart = TextIndex;
		if ((character >= 'a' && character <= 'z') || (character >= 'A' && character <= 'Z'))
		{
			while (TextIndex < app->TextCount)
			{
				rune next_character = app->Text[TextIndex];
				if ((next_character >= 'a' && next_character <= 'z') ||
					(next_character >= 'A' && next_character <= 'Z') ||
					(next_character >= '0' && next_character <= '9') ||
					(next_character == '_'))
				{
					TextIndex++;
				}
				else
				{
					break;
				}
			}

			NewToken->Type = TokenIdentifier;
		}
		else if (character >= '0' && character <= '9')
		{
			while (TextIndex < app->TextCount &&
				   app->Text[TextIndex] >= '0' && app->Text[TextIndex] <= '9')
			{
				TextIndex++;
			}
			NewToken->Type = TokenNumber;
		}
		else
		{
			TextIndex++;
			NewToken->Type = (TokenType)character;
		}

		s32 TokenEnd		  = TextIndex;
		NewToken->Lexeme.Data = (u8 *)&app->Text[TokenStart];
		NewToken->Lexeme.Size = (umm)(TokenEnd - TokenStart);
		umm Size			  = NewToken->Lexeme.Size;

		switch (NewToken->Lexeme.Data[TokenStart + Size])
		{
			case ('('):
			{
				NewToken->Type = (TokenType)'(';
				break;
			}

			case (')'):
			{
				NewToken->Type = (TokenType)')';
				break;
			}

			case ('{'):
			{
				NewToken->Type = (TokenType)'{';
				break;
			}

			case ('}'):
			{
				NewToken->Type = (TokenType)'}';
				break;
			}

			case (','):
			{
				NewToken->Type = (TokenType)',';
				break;
			}

			case (';'):                            
			{
				NewToken->Type = (TokenType)';';
				break;
			}

			case ('+'):
			{
				NewToken->Type = (TokenType)'+';
				break;
			}

			case '-':
			{
				if (NewToken->Lexeme.Data[TokenStart + Size + 1] == '>')
				{
					NewToken->Type = (TokenType)'>';
					Size++;
				}
				else
				{
					NewToken->Type = (TokenType)'-';
				}
				break;
			}

			case '*':
			{
				NewToken->Type = (TokenType)'*';
				break;
			}

			case '/':
			{
				NewToken->Type = (TokenType)'/';
				break;
			}

			case '=':
			{
				if (NewToken->Lexeme.Data[TokenStart + Size + 1] == '=')
				{
					NewToken->Type = TokenDoubleEqual;
					Size++;
				}
				else
				{
					NewToken->Type = (TokenType)'=';
				}
				break;
			}

			case '>':
			{
				if (NewToken->Lexeme.Data[TokenStart + Size + 1] == '=')
				{
					NewToken->Type = TokenGreaterEqual;
					Size++;
				}
				else
				{
					NewToken->Type = (TokenType)'>';
				}
				break;
			}

			case '<':
			{
				if (NewToken->Lexeme.Data[TokenStart + Size + 1] == '=')
				{
					NewToken->Type = TokenLesserEqual;
					Size++;
				}
				else
				{
					NewToken->Type = (TokenType)'<';
				}
				break;
			}

			case ' ':
			case '\t':
			case '\n':
			case '\r':
			{
				NewToken->Type = TokenWhiteSpace;
				break;
			}

			default:
			{
				NewToken->Type = TokenUndefined;
				break;
			}
		}

		NewToken->Next = 0;

		if (!List->Root)
		{
			List->Root = NewToken;
			List->Leaf = NewToken;
		}
		else
		{
			List->Leaf->Next = NewToken;
			List->Leaf		 = NewToken;
		}

		List->Count++;

		Column += (s32)NewToken->Lexeme.Size;
	}

	return List;
}
