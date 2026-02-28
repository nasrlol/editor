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
  for(s32 index = 0;
  index < sizeof(delimiters);
  ++index)
  {
    if(delimiters[index] == Character)
    {
      return 1;
    }
  }
  return 0;
}

internal token_list *
Lex(app_state *App, arena *Arena)
{
  token_list *List        = PushStruct(Arena, token_list);
  b32         Initialized = 0;

  s32 Line   = 1;
  s32 Column = 1;

  for(s32 TextIndex = 0; TextIndex < App->TextCount; TextIndex++)
  {
    rune Character = App->Text[TextIndex];

    if(Character == ' ' || Character == '\t')
    {
      Column++;
      continue;
    }
    if(Character == '\n' || Character == '\r' || Character == ';')
    {
      Line++;
      Column = 1;
      continue;
    }

    token_node *TokenNode = PushStruct(Arena, token_node);
    token      *Token     = PushStruct(Arena, token);
    TokenNode->Token      = Token;

    Token->Line       = Line;
    Token->Column     = Column;
    Token->ByteOffset = (u64)TextIndex;

    s32 TokenStart = TextIndex;

    if(!Is_Delimiter(Character))
    {
      Token->Type = (token_type)Character;
      break;
    }

    while(TextIndex + 1 < App->TextCount && Is_Alpha_Num(App->Text[TextIndex + 1]))
    {
      TextIndex++;
    }

      //TODO(nasr): here, we werre simplifying everything here and cleaning up bugs 

    // +1 because we want to access the index + 1
    if(Is_Alpha(Character)) Token->Type = TokenIdentifier;
    else if(Is_Digit(Character)) Token->Type = TokenNumber;

    else
    {
      if(Character > 126)
      {
        Token->Type = TokenUnwantedChild;
      }

      else
      {
        rune Next = (TextIndex + 1 < App->TextCount) ? App->Text[TextIndex + 1] : 0;

        switch(Character)
        {
          case '=':
            if(Next == '=')
              Token->Type = TokenDoubleEqual;
            break;
          case '>':
            if(Next == '=')
              Token->Type = TokenGreaterEqual;
            else if(Next == '>')
              Token->Type = TokenRightShift;
            break;
          case '<':
            if(Next == '=')
              Token->Type = TokenLesserEqual;
            else if(Next == '<')
              Token->Type = TokenLeftShift;
            break;
          default:
            Token->Type = (token_type)Character;
            break;
        }
      }
      TextIndex++;
    }

    s32 TokenEnd       = TextIndex + 1;
    Token->Lexeme.Data = (u8 *)&App->Text[TokenStart];
    Token->Lexeme.Size = (u64)(TokenEnd - TokenStart) * sizeof(rune);

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

    Column += (s32)Token->Lexeme.Size;
  }

  return List;
}
