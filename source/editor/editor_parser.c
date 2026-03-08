internal syntax_node *
CreateNode(concrete_syntax_tree *Tree, arena *Arena, token *Token)
{
    syntax_node *SyntaxNode = PushStruct(Arena, syntax_node);
    SyntaxNode->Token       = Token;
    SyntaxNode->First       = &nil_syntax_node;
    SyntaxNode->Last        = &nil_syntax_node;
    SyntaxNode->Next    = &nil_syntax_node;
    SyntaxNode->Parent      = &nil_syntax_node;
    return SyntaxNode;
}

internal syntax_node *
PeekToOffset(syntax_node *Node, s32 PeekOffset)
{
    for(s32 PeekCount = 0; PeekCount < PeekOffset; PeekCount++)
    {
        if(!Node || !Node->Next)
        {
            return &nil_syntax_node;
        }
        Node = Node->Next;
    }
    return Node;
}

internal void
NodePushChild(concrete_syntax_tree *Tree, syntax_node *Node)
{
    Node->Parent = Tree->Current;

    if(Tree->Current->First == &nil_syntax_node)
    {
        Tree->Current->First = Node;
        Tree->Current->Last  = Node;
    }
    else
    {
        Tree->Current->Last->Next = Node;
        Tree->Current->Last           = Node;
    }
}

internal inline b32
IsNilSyntaxNode(syntax_node *Node)
{
    return Node == &nil_syntax_node || Node == NULL;
}

internal void
DisownNode(concrete_syntax_tree *Tree, syntax_node *ParentNode, syntax_node *ChildNode)
{
    // TODO(nasr)
}

internal void
AdoptNode(concrete_syntax_tree *Tree, syntax_node *ParentNode, syntax_node *ChildNode)
{
    // TODO(nasr)
}

internal inline void
MarkDirty(token *Token)
{
    Token->Flags = (token_flags)(Token->Flags | FlagDirty);
}

internal concrete_syntax_tree *
Parse(arena *Arena, token_list *List, concrete_syntax_tree *Tree)
{
    for(token_node *TokenNode = List->Root;
        TokenNode != &nil_token_node && TokenNode != NULL;
        TokenNode = TokenNode->Next)
    {
        token       *Token      = TokenNode->Token;
        syntax_node *SyntaxNode = CreateNode(Tree, Arena, Token);

        if(IsNilSyntaxNode(Tree->Root))
        {
            Tree->Root    = SyntaxNode;
            Tree->Current = SyntaxNode;
        }

        switch((token_type)Token->Type)
        {
            case TokenIdentifier:
            {
                if(!IsNilTokenNode(TokenNode) && TokenNode->Next->Token->Type == (token_type)'=')
                {

					token_node *ValueNode = TokenNode->Next->Next;

                    if(ValueNode->Token->Type != TokenIdentifierAssignmentValue &&
                       ValueNode->Token->Type != TokenValue)
                    {
                        ValueNode->Token->Type = TokenIdentifierAssignmentValue;
                    }
                }

                if(Tree->Current != SyntaxNode)
                {
                    NodePushChild(Tree, SyntaxNode);
                }
            }
            break;

            case TokenIdentifierAssignmentValue:
            {
                NodePushChild(Tree, SyntaxNode);
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

                NodePushChild(Tree, SyntaxNode);
            }
            break;

            case TokenDoubleEqual:
            {
                // TODO(nasr): busy
                syntax_node *LeftOperand  = Tree->Current->Last;
                syntax_node *RightOperand = Tree->Current->First;

                NodePushChild(Tree, SyntaxNode);
                Tree->Current = SyntaxNode;

                if(!IsNilSyntaxNode(LeftOperand) && !IsNilSyntaxNode(RightOperand))
                {
                    Tree->Current->First = LeftOperand;
                    Tree->Current->Last  = RightOperand;

                    LeftOperand->Parent   = Tree->Current;
                    LeftOperand->Next = &nil_syntax_node;

                    RightOperand = Tree->Current->Next;
                }
            }
            break;

            case TokenGreaterEqual:
            case TokenLesserEqual:
            case(token_type)'<':
            case(token_type)'>':
            {
                NodePushChild(Tree, SyntaxNode);
                Tree->Current = SyntaxNode;
            }
            break;

            case(token_type)'(':
            {
                NodePushChild(Tree, SyntaxNode);
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

                NodePushChild(Tree, SyntaxNode);
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
                // reposition the current node to the parent ndoe
                // define the last of the childs to be the current node

                Tree->Current->Last = Tree->Current;
                Tree->Current       = Tree->Current->Parent;
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

                NodePushChild(Tree, SyntaxNode);
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

                NodePushChild(Tree, SyntaxNode);
                Tree->Current = SyntaxNode;
            }
            break;

            case TokenIf:
            {
                NodePushChild(Tree, SyntaxNode);
                Tree->Current = SyntaxNode;
            }
            break;

            case TokenElse:
            case TokenWhile:
            case TokenFor:
            {
                NodePushChild(Tree, SyntaxNode);
                Tree->Current = SyntaxNode;
            }
            break;

            case TokenBreak:
            {
                if(!Tree->Current || !Tree->Current->Parent)
                {
                    MarkDirty(Token);
                }
                NodePushChild(Tree, SyntaxNode);
            }
            break;

            case TokenContinue:
            {
                NodePushChild(Tree, SyntaxNode);
            }
            break;

            case TokenExpression:
            {
                NodePushChild(Tree, SyntaxNode);
                Tree->Current = SyntaxNode;
            }
            break;

            case TokenParam:
            {
                NodePushChild(Tree, SyntaxNode);
            }
            break;

            case TokenStar:
            {
                // TODO(nasr): once we get to better visualizations i think
            }
            break;

            case TokenUndefined:
            {
                MarkDirty(Token);
                NodePushChild(Tree, SyntaxNode);
            }
            break;

            default:
            {
                MarkDirty(Token);
                NodePushChild(Tree, SyntaxNode);
            }
            break;
        }
    }

    return Tree;
}
