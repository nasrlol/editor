internal syntax_node *
Peek(syntax_node *Node, token_type Type)
{
    for (; Node; Node = Node->NextNode)
    {
        if (Node->Token->Type == Type)
        {
            return Node;
        }
    }
    return 0;
}

internal concrete_syntax_tree *
Parse(arena *Arena, token_list *List)
{
 concrete_syntax_tree *Tree = PushStruct(Arena, concrete_syntax_tree);
 MemoryZero(Tree);

 for (token_node *Node = List->Root; Node != 0; Node = Node->Next)
 {
  token *Token = Node->Token;

  syntax_node *SyntaxNode = PushStruct(Arena, syntax_node);
  SyntaxNode->Token       = Token;

  if (!Tree->Root)
  {
   Tree->Root    = SyntaxNode;
   Tree->Current = SyntaxNode;
   continue;
  }

  // Append child
  {
   syntax_node *Parent = Tree->Current;
   syntax_node *Child  = SyntaxNode;

   if (!Parent) continue;
   Child->Parent = Parent;

   // TODO(nasr): segfaults for some reason
   if (Parent->First == NULL)
   {
    Parent->First = Child;
    Parent->Last  = Child;
   }
   else
   {
    Parent->Last->NextNode = Child;
    Parent->Last           = Child;
   }
  }

  switch ((token_type)Token->Type)
  {
   case (TokenUndefined):
   {
    // mark dirty for incremental parsing later
    // TODO(nasr): if we ever implement incremental parsing, search
    // for dirty flags and attempt fixup
    Token->Flags = (token_flags)(Token->Flags | FlagDirty);
   }
   break;

   case (TokenIdentifier):
   {
    // if followed by '=' this is an assignment
    // the token after '=' is the value so bassically a double equal
    if (Node->Next && Node->Next->Token->Type == (token_type)'=')
    {
     token_node *ValueNode = Node->Next->Next;
     if (ValueNode)
     {
      ValueNode->Token->Type = TokenIdentifierValue;
     }
    }
   }
   break;

   case (TokenIdentifierValue):
   {
    // TODO(nasr): validate that preceding nodes are well-formed
   }
   break;

   case (TokenNumber):
   case (TokenString):
   {
    if (Tree->Current->Token->Type == (token_type)'=')
    {
     Token->Type = TokenIdentifierValue;

     // if the parent of '=' is not an identifier, something is wrong
     if (Tree->Current->Parent &&
         Tree->Current->Parent->Token->Type != TokenIdentifier)
     {
      Token->Flags = (token_flags)(Token->Flags | FlagDirty);
     }
    }
    // NOTE(nasr): number/string inside a block body -- no action yet
    // TODO(nasr): figure out what to do here
   }
   break;

   case (TokenDoubleEqual):
   case (TokenGreaterEqual):
   case (TokenLesserEqual):
   case ((token_type)'<'):
   case ((token_type)'>'):
   {
    // comparison operators -- children will be lhs and rhs
    // descend so subsequent tokens attach as children
    Tree->Current = SyntaxNode;
   }
   break;

   case ((token_type)'{'):
   {
    Tree->Current = SyntaxNode;
   }
   break;

   case ((token_type)'}'):
   {
    while (Tree->Current &&
           Tree->Current->Token->Type != (token_type)'{')
    {
     Tree->Current = Tree->Current->Parent;
    }

    if (Tree->Current)
    {
     Tree->Current = Tree->Current->Parent;
    }
   }
   break;

   case ((token_type)'('):
   {
    Tree->Current = SyntaxNode;
   }
   break;

   case ((token_type)')'):
   {
    while (Tree->Current &&
           Tree->Current->Token->Type != (token_type)'(')
    {
     Tree->Current = Tree->Current->Parent;
    }

    if (Tree->Current)
    {
     Tree->Current = Tree->Current->Parent;
    }
   }
   break;

   case (TokenParam):
   {
   }
   break;

   case (TokenFunc):
   case (TokenReturn):
   case (TokenIf):
   case (TokenElse):
   case (TokenWhile):
   case (TokenFor):
   {
    Tree->Current = SyntaxNode;
   }
   break;

   case (TokenBreak):
   {
    // break must be inside a loop or switch if no parent, mark dirty
    if (!Tree->Current->Parent)
    {
     Token->Flags = (token_flags)(Token->Flags | FlagDirty);
    }
   }
   break;

   case (TokenContinue):
   {
    // TODO(nasr): same validation as break
   }
   break;

   case (TokenExpression):
   {
    Tree->Current = SyntaxNode;
   }
   break;
  }
 }

 return Tree;
}
