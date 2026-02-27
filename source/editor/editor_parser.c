internal syntax_node *
CreateNode(concrete_syntax_tree *Tree, arena *Arena, token *Token)
{
  syntax_node *SyntaxNode = PushStruct(Arena, syntax_node);
  SyntaxNode->Token       = Token;

  return SyntaxNode;
}

internal syntax_node *
PeekToOffset(syntax_node *Node, s32 peekOffset)
{
  for (s32 peekCount = 0; peekCount < peekOffset; peekCount++) Node = Node->NextNode;
  if (!Node) return &nil_syntax_node;

  return Node;
}
                                                                         
internal syntax_node *
PeekForward(syntax_node *Node, token_type type)
{
  while (Node->Token->Type != type) Node = Node->NextNode;
  return Node;
}

/* pushing parsed node on on the syntax tree */
internal void
NodePush(concrete_syntax_tree *Tree, syntax_node *Node)
{
  assert(Tree);
  assert(Node);

  if (!Tree->Root)
  {
    Tree->Root = Node;
    Tree->Current = Tree->Root;
  }

  syntax_node *Parent = Tree->Current;
  syntax_node *Child  = Node;


  if(!Parent->First)
  {
    Parent->First = Child;
    Parent->Last  = Child;
  }
  else
  {
    Parent->Last->NextNode = Child;
    Parent->Last           = Child;
  }

  Child->Parent = Parent;
}

internal concrete_syntax_tree *
Parse(arena *Arena, token_list *List)
{
  concrete_syntax_tree *Tree = PushStruct(Arena, concrete_syntax_tree);

  for (token_node *Node = List->Root; Node != 0; Node = Node->Next)
  {
    token *Token = Node->Token;
    // creating the node in a seperaate function
    // 1. this is an action that requires multiple steps
    // 2. this is an action that is going to be repeated quite a few times
    
    syntax_node *SyntaxNode = CreateNode(Tree, Arena, Token);
    Assert(SyntaxNode);

    // TODO(nasr): passing the syntax node here gives a null pointed parent
    // and a null pointed root, but when creating the syntax node it's fine
    NodePush(Tree, SyntaxNode);

    switch ((token_type)Token->Type)
    {
      case (TokenUndefined):
        {
          // TODO(nasr): if we ever implement incremental parsing, search
          // mark dirty for incremental parsing later
          // for dirty flags and attempt fixup
          Token->Flags = (token_flags)(Token->Flags | FlagDirty);
        }
        break;

      case (TokenIdentifier):
        {
          // if followed by '=' this is an assignment
          // the token after '=' is the value so bassically a double equal
          // check if the next node exists
          if (Node->Next && Node->Next->Token->Type == (token_type)'=')
          {
            token_node *ValueNode = Node->Next->Next;
            if (ValueNode)
            {
              ValueNode->Token->Type = TokenIdentifierAssignmentValue;
            }
          }

          if (!Is_Alpha((Node->Token->Lexeme.Data[0]))) Node->Token->Flags = (token_flags)(Token->Flags | FlagDirty);
        }
        break;

      case (TokenIdentifierAssignmentValue):
        {
          // TODO(nasr): validate that preceding nodes are well-formed
          break;
        }

      case (TokenNumber):
      case (TokenString):
        {
          if (Tree->Current->Token->Type == (token_type)'=')
          {
            Token->Type = TokenIdentifierAssignmentValue;

            // if the parent of '=' is not an identifier, something is wrong
            if (Tree->Current->Parent &&
                Tree->Current->Parent->Token->Type != TokenIdentifier)
            {
              Token->Flags = (token_flags)(Token->Flags | FlagDirty);
            }
          }

          // NOTE(nasr): number/string inside a block body -- no action yet
          // TODO(nasr): figure out what to do here
          break;
        }

      case (TokenDoubleEqual):
        {
          /* if (1==1) */

          // TODO(nasr): take the parent if its a '('
          if (Tree->Current->Parent->Token->Type != (token_type)'(')
          {
            break;
          }

          // TODO(nasr): handle comparison operators

          Tree->Current = SyntaxNode;
          break;
        }

      case (TokenGreaterEqual):
      case (TokenLesserEqual):
      case ((token_type)'<'):
      case ((token_type)'>'):
        {
          token_type CurrentType = Tree->Current->Parent->Token->Type;

          if (CurrentType == TokenIdentifier || CurrentType == TokenIdentifierAssignmentValue)
          {
          }

          Tree->Current = SyntaxNode;
          break;
        }

      case ((token_type)'{'):
        {
          if (Tree->Current && Tree->Current != Tree->Root)
          {
            Tree->Current = Tree->Current->Parent;
          }

          break;
        }

      case ((token_type)'}'):
        {
          if (Tree->Current->Parent)
          {
            Tree->Current = Tree->Current->Parent;
          }
          break;
        }

      case ((token_type)'('):
        {
          Tree->Current = SyntaxNode;
          break;
        }

      case ((token_type)')'):
        {
          while (Tree->Current && Tree->Current->Token->Type != (token_type)'(')
          {
            Tree->Current = Tree->Current->Parent;
          }

          if (Tree->Current)
          {
            Tree->Current = Tree->Current->Parent;
          }
          break;
        }

      case (TokenParam):
        {
          break;
        }

      case (TokenFunc):
        {
          syntax_node *current_node = Tree->Current;
          if (current_node->NextNode && current_node->NextNode->Token->Type != (token_type)'{')
          {
            Tree->Current->Token->Flags = (token_flags)(Token->Flags | FlagDirty);
            break;
          }
        }

      case (TokenReturn):
        {
          if (Tree->Current->Parent && Tree->Current->Parent->Token->Type != TokenFunc)
          {
          }
        }

      case (TokenIf):
        {
          if (Tree->Current->Parent &&
              (Tree->Current->Parent->Token->Type != (token_type)'(' &&
               Tree->Current->Parent->Token->Type != (token_type)')'))
          {
          }
        }

      case (TokenElse):
      case (TokenWhile):
      case (TokenFor):
        {
          Tree->Current = SyntaxNode;

          break;
        }

      case (TokenBreak):
        {
          // break must be inside a loop or switch if no parent, mark dirty
          if (!Tree->Current->Parent)
          {
            Token->Flags = (token_flags)(Token->Flags | FlagDirty);
          }
          break;
        }

      case (TokenContinue):
        {
          // TODO(nasr): same validation as break
          break;
        }

      case (TokenExpression):
        {
          Tree->Current = SyntaxNode;
          break;
        }

      case ((token_type)';'):
        {
          // TODO(nasr): do something to make the end of line clear
          Tree->Current = SyntaxNode;
          break;
        }

      default:
        {
          Tree->Current->Token->Flags = (token_flags)(Token->Flags | FlagDirty);
          break;
        }
    }
  }

  return Tree;
}
