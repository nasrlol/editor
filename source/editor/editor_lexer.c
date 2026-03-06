internal b32
Is_Alpha(rune Character)
{
  return ((Character >= 'a' && Character <= 'z') ||
          (Character >= 'A' && Character <= 'Z') ||
          (Character == '_'));
}

internal b32
Is_Digit(rune Character)
{
  return (Character >= '0' && Character <= '9');
}

internal b32
Is_Alpha_Num(rune Character)
{
  return (Is_Alpha(Character) || Is_Digit(Character));
}

internal b32
Is_Delimiter(rune Character)
{
  for(s32 Index = 0; Index < ArrayCount(Delimiters); ++Index)
  {
    if(Delimiters[Index] == Character)
    {
      return 1;
    }
  }
  return 0;
}

internal b32
Is_WhiteSpace(rune Character)
{
  return (Character == '\n' || Character == '\r' ||
          Character == ' ' || Character == '\t');
}

internal b32
Is_TokenBreak(rune Character)
{
  return (Is_WhiteSpace(Character) || Is_Delimiter(Character));
}

internal token_list *
Lex(app_state *App, arena *Arena)
{
  token_list *List        = PushStruct(Arena, token_list);
  b32         Initialized = 0;
  s32         Line        = 1;
  s32         Column      = 1;

  for(s32 TextIndex = 0; TextIndex < App->TextCount; TextIndex++)
  {
    rune Character = App->Text[TextIndex];

    if(Character == '\r' || Character == '\n')
    {
      if(Character == '\r' &&
         (TextIndex + 1 < App->TextCount) &&
         App->Text[TextIndex + 1] == '\n')
      {
        TextIndex++;
      }

      TextIndex++;

      Line++;
      Column = 1;
      continue;
    }

    if(Is_WhiteSpace(Character))
    {
      Column++;
      continue;
    }

    token_node *TokenNode = PushStruct(Arena, token_node);
    token      *Token     = PushStruct(Arena, token);
    TokenNode->Token      = Token;
    Token->Line           = Line;
    Token->Column         = Column;
    Token->ByteOffset     = (u64)TextIndex;
    Token->Flags          = FlagNone;

    s32 TokenStart = TextIndex;
    s32 TokenEnd   = TextIndex;

    if(Character > 126)
    {
      Token->Type = TokenUnwantedChild;
      TokenEnd    = TextIndex + 1;
    }
    else if(Is_Alpha(Character))
    {
      while((TextIndex + 1 < App->TextCount) &&
            Is_Alpha_Num(App->Text[TextIndex + 1]))
      {
        TextIndex++;
      }
      TokenEnd    = TextIndex + 1;
      Token->Type = TokenIdentifier;
    }
    else if(Is_Digit(Character))
    {
      while((TextIndex + 1 < App->TextCount) &&
            Is_Digit(App->Text[TextIndex + 1]))
      {
        TextIndex++;
      }
      TokenEnd    = TextIndex + 1;
      Token->Type = TokenNumber;
    }
    else
    {
      rune Next = (TextIndex + 1 < App->TextCount) ? App->Text[TextIndex + 1] : 0;

      switch(Character)
      {
        case '=':
        {
          if(Next == '=')
          {
            Token->Type = TokenDoubleEqual;
            TextIndex++;
          }
          else
          {
            Token->Type = (token_type)'=';
          }
          break;
        }

        case '>':
        {
          if(Next == '=')
          {
            Token->Type = TokenGreaterEqual;
            TextIndex++;
          }
          else if(Next == '>')
          {
            Token->Type = TokenRightShift;
            TextIndex++;
          }
          else
          {
            Token->Type = (token_type)'>';
          }
          break;
        }

        case '<':
        {
          if(Next == '=')
          {
            Token->Type = TokenLesserEqual;
            TextIndex++;
          }
          else if(Next == '<')
          {
            Token->Type = TokenLeftShift;
            TextIndex++;
          }
          else
          {
            Token->Type = (token_type)'<';
          }
          break;
        }

        default:
        {
          Token->Type = (token_type)Character;
          break;
        }
      }

      TokenEnd = TextIndex + 1;
    }

    Token->Lexeme.Data = (u8 *)&App->Text[TokenStart];
    Token->Lexeme.Size = (u64)(TokenEnd - TokenStart);
    Column += (s32)Token->Lexeme.Size;

    if(!Initialized)
    {
      Initialized   = 1;
      List->Root    = TokenNode;
      List->Current = TokenNode;
    }
    else
    {
      TokenNode->Previous = List->Current;
      List->Current->Next = TokenNode;
      List->Current       = TokenNode;
    }
  }

  return List;
}
