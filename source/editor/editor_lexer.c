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

internal token_list *
Lex(app_state *App, arena *Arena)
{
 token_list *List        = PushStruct(Arena, token_list);
 b32         Initialized = 0;

 s32 Line   = 1;
 s32 Column = 1;

 for (s32 TextIndex = 0; TextIndex < App->TextCount; TextIndex++)
 {
  rune Character = App->Text[TextIndex];

  if (Character == ' ' || Character == '\t')
  {
   Column++;
   continue;
  }
  if (Character == '\n' || Character == '\r')
  {
   Line++;
   Column = 1;
   continue;
  }

  token_node *Node  = PushStruct(Arena, token_node);
  token      *Token = PushStruct(Arena, token);
  Node->Token       = Token;

  Token->Line       = Line;
  Token->Column     = Column;
  Token->ByteOffset = (u64)TextIndex;

  s32 TokenStart = TextIndex;

  if (Is_Alpha(Character))
  {
   while (TextIndex + 1 < App->TextCount &&
          Is_Alpha_Num(App->Text[TextIndex + 1]))
   {
    TextIndex++;
   }
   Token->Type = TokenIdentifier;
  }

  else if (Is_Digit(Character))
  {
   while (TextIndex + 1 < App->TextCount &&
          Is_Digit(App->Text[TextIndex + 1]))

   {
    TextIndex++;
   }

   Token->Type = TokenNumber;
  }

  else
  {
   if (Character > 126)
   {
    Token->Type = TokenUnwantedChild;
   }
   else
   {
    rune Next = (TextIndex + 1 < App->TextCount) ? App->Text[TextIndex + 1] : 0;

    switch (Character)
    {
     case '=':
     {
      if (Next == '=')
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
      if (Next == '=')
      {
       Token->Type = TokenGreaterEqual;
       TextIndex++;
      }
      else if (Next == '>')
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
      if (Next == '=')
      {
       Token->Type = TokenLesserEqual;
       TextIndex++;
      }
      else if (Next == '<')
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
   }

   // convert utf8 size to rune
   // and calculate lexeme sizes
   s32 TokenEnd       = TextIndex + 1;
   Token->Lexeme.Data = (u8 *)&App->Text[TokenStart];
   ////////////////////////////////////////////////////////////////////
   // suggested fix for utf8 multiplying the size of the thing by the
   // size of rune so we have fitting stuff
   // [failed]
   //////////////////////////////////////////////////////
   // TODO(nasr): convert all of this to utf8 handling
   Token->Lexeme.Size = (u64)(TokenEnd - TokenStart) * sizeof(rune);

   if (!Initialized)
   {
    Initialized   = 1;
    List->Root    = Node;
    List->Current = Node;
   }
   else
   {
    Node->Previous      = List->Current;
    List->Current->Next = Node;
    List->Current       = Node;
   }

   Column += (s32)Token->Lexeme.Size;
  }
 }

 return List;
}
