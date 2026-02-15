internal TokenList *
Lex(app_state *app, arena *Arena)
{
	TokenList *List		 = PushStruct(Arena, TokenList);
	s32		   TextIndex = 0;
	s32		   Line		 = 1;
	s32		   Column	 = 1;

	while (TextIndex < app->TextCount)
	{
		rune ch = app->Text[TextIndex];

		if (ch == ' ' || ch == '\t')
		{
			TextIndex++;
			Column++;
			continue;
		}
		if (ch == '\n' || ch == '\r')
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
		if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z'))
		{
			while (TextIndex < app->TextCount)
			{
				rune next_ch = app->Text[TextIndex];
				if ((next_ch >= 'a' && next_ch <= 'z') ||
					(next_ch >= 'A' && next_ch <= 'Z') ||
					(next_ch >= '0' && next_ch <= '9') ||
					(next_ch == '_'))
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
		else if (ch >= '0' && ch <= '9')
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
			NewToken->Type = (TokenType)ch;
		}

		s32 TokenEnd		  = TextIndex;
		NewToken->Lexeme.Data = (u8 *)&app->Text[TokenStart];
		NewToken->Lexeme.Size = (umm)(TokenEnd - TokenStart);
        umm Size =  NewToken->Lexeme.Size;

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
