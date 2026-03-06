internal syntax_node *
CreateNode(concrete_syntax_tree *Tree, arena *Arena, token *Token)
{
  syntax_node *SyntaxNode = PushStruct(Arena, syntax_node);
  SyntaxNode->Token       = Token;
  SyntaxNode->First       = &nil_syntax_node;
  SyntaxNode->Last        = &nil_syntax_node;
  SyntaxNode->NextNode    = &nil_syntax_node;
  SyntaxNode->Parent      = &nil_syntax_node;
  return SyntaxNode;
}

internal syntax_node *
PeekToOffset(syntax_node *Node, s32 PeekOffset)
{
  for(s32 PeekCount = 0; PeekCount < PeekOffset; PeekCount++)
  {
    if(!Node || !Node->NextNode)
    {
      return &nil_syntax_node;
    }
    Node = Node->NextNode;
  }
  return Node;
}

internal syntax_node *
PeekForward(syntax_node *Node, token_type Type)
{
  while(Node && Node != &nil_syntax_node)
  {
    if(Node->Token->Type == Type)
    {
      return Node;
    }
    Node = Node->NextNode;
  }
  return &nil_syntax_node;
}

internal void
NodePush(concrete_syntax_tree *Tree, syntax_node *Node)
{
  assert(Tree);
  assert(Node);

  if(Tree->Root == &nil_syntax_node || Tree->Root == NULL)
  {
    Tree->Root    = Node;
    Tree->Current = Node;
    return;
  }

  syntax_node *Current = Tree->Current;
  Node->Parent         = Current;

  if(Current->First == &nil_syntax_node)
  {
    Current->First = Node;
    Current->Last  = Node;
  }
  else
  {
    Current->Last->NextNode = Node;
    Current->Last           = Node;
  }
}

internal void
DisownNode(concreete_syntax_tree *Tree, syntax_node *ParentNode, syntax_node *ChildNode)
{
}

internal void
AdoptNode(concrete_syntax_tree *Tree, syntax_node *ParentNode, syntax_node *ChildNode)
{
}

internal inline void
MarkDirty(token *Token)
{
  Token->Flags = (token_flags)(Token->Flags | FlagDirty);
}

internal concrete_syntax_tree *
Parse(arena *Arena, token_list *List)
{
  concrete_syntax_tree *Tree = PushStructZero(Arena, concrete_syntax_tree);

  for(token_node *TokenNode = List->Root; TokenNode != &nil_token_node && TokenNode != NULL; TokenNode = TokenNode->Next)
  {
    token       *Token      = TokenNode->Token;
    syntax_node *SyntaxNode = CreateNode(Tree, Arena, Token);

    switch((token_type)Token->Type)
    {
      case TokenIdentifier:
      {
        if(!Is_Alpha(Token->Lexeme.Data[0]))
        {
          MarkDirty(Token);
        }

        if(TokenNode->Next &&
           TokenNode->Next->Token->Type == (token_type)'=')
        {
          token_node *ValueNode = TokenNode->Next->Next;
          if(ValueNode)
          {
            ValueNode->Token->Type = TokenIdentifierAssignmentValue;
          }
        }

        NodePush(Tree, SyntaxNode);
      }
      break;

      case TokenIdentifierAssignmentValue:
      {
        NodePush(Tree, SyntaxNode);
      }
      break;

      case TokenNumber:
      case TokenString:
      {
        if(Tree->Current && Tree->Current->Token->Type == (token_type)'=')
        {
          Token->Type = TokenIdentifierAssignmentValue;

          if(Tree->Current->Parent && Tree->Current->Parent->Token->Type != TokenIdentifier)
          {
            MarkDirty(Token);
          }
        }

        NodePush(Tree, SyntaxNode);
      }
      break;

      case TokenDoubleEqual:
      {
        if(Tree->Current &&
           Tree->Current->Parent &&
           Tree->Current->Parent->Token->Type != (token_type)'(')
        {
        }

        NodePush(Tree, SyntaxNode);
        Tree->Current = SyntaxNode;
      }
      break;

      case TokenGreaterEqual:
      case TokenLesserEqual:
      case(token_type)'<':
      case(token_type)'>':
      {
        NodePush(Tree, SyntaxNode);
        Tree->Current = SyntaxNode;
      }
      break;

      case(token_type)'(':
      {
        NodePush(Tree, SyntaxNode);
        Tree->Current = SyntaxNode;
      }
      break;

      case(token_type)')':
      {
        while(Tree->Current &&
              Tree->Current != &nil_syntax_node &&
              Tree->Current->Token->Type != (token_type)'(')
        {
          Tree->Current = Tree->Current->Parent;
        }

        if(Tree->Current && Tree->Current->Parent)
        {
          Tree->Current = Tree->Current->Parent;
        }
      }
      break;

      case(token_type)'{':
      {
        if(Tree->Current && Tree->Current != Tree->Root)
        {
          Tree->Current = Tree->Current->Parent;
        }

        NodePush(Tree, SyntaxNode);
        Tree->Current = SyntaxNode;
      }
      break;

      case(token_type)'}':
      {
        if(Tree->Current && Tree->Current->Parent)
        {
          Tree->Current = Tree->Current->Parent;
        }
      }
      break;

      case(token_type)';':
      {
        NodePush(Tree, SyntaxNode);
        if(Tree->Current && Tree->Current->Parent)
        {
          Tree->Current = Tree->Current->Parent;
        }
      }
      break;

      case TokenFunc:
      {
        b32 HasBody = 0;
        for(token_node *Lookahead = TokenNode->Next;
        Lookahead != NULL;
        Lookahead = Lookahead->Next)
        {
          token_type LT = Lookahead->Token->Type;
          if(LT == (token_type)'{')
          {
            HasBody = 1;
            break;
          }
          if(LT == (token_type)';')
          {
            break;
          }
        }

        if(!HasBody)
        {
          MarkDirty(Token);
        }

        NodePush(Tree, SyntaxNode);
        Tree->Current = SyntaxNode;
      }
      break;

      case TokenReturn:
      {
        if(Tree->Current &&
           Tree->Current->Parent &&
           Tree->Current->Parent->Token->Type != TokenFunc)
        {
        }

        NodePush(Tree, SyntaxNode);
        Tree->Current = SyntaxNode;
      }
      break;

      case TokenIf:
      {
        NodePush(Tree, SyntaxNode);
        Tree->Current = SyntaxNode;
      }
      break;

      case TokenElse:
      case TokenWhile:
      case TokenFor:
      {
        NodePush(Tree, SyntaxNode);
        Tree->Current = SyntaxNode;
      }
      break;

      case TokenBreak:
      {
        if(!Tree->Current || !Tree->Current->Parent)
        {
          MarkDirty(Token);
        }

        NodePush(Tree, SyntaxNode);
      }
      break;

      case TokenContinue:
      {
        NodePush(Tree, SyntaxNode);
      }
      break;

      case TokenExpression:
      {
        NodePush(Tree, SyntaxNode);
        Tree->Current = SyntaxNode;
      }
      break;

      case TokenParam:
      {
        NodePush(Tree, SyntaxNode);
      }
      break;

      case TokenUndefined:
      {
        MarkDirty(Token);
        NodePush(Tree, SyntaxNode);
      }
      break;

      default:
      {
        MarkDirty(Token);
        NodePush(Tree, SyntaxNode);
      }
      break;
    }
  }

  return Tree;
}
