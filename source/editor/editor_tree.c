internal v_node *
CreateVNode(arena *Arena, syntax_node *Node)
{
  v_node *VNode = PushStructZero(Arena, v_node);
  VNode->Color  = {0};
  VNode->Size   = {10.f};
  VNode->Pos    = {0};
  // VNode->Label     = Node->Token->Lexeme;
  VNode->Label     = {NULL, 0};
  VNode->Parent    = &nil_v_node;
  VNode->NextVNode = &nil_v_node;
  VNode->First     = &nil_v_node;
  VNode->Last      = &nil_v_node;
  return VNode;
}

internal debug_tree *
BuildDebugTree(concrete_syntax_tree *Tree, arena *Arena)
{
  debug_tree *DebugTree = PushStructZero(Arena, debug_tree);

  for(syntax_node *Node = Tree->Root;
  Node && Node != &nil_syntax_node;
  Node = Node->First)
  {
    v_node *VNode = CreateVNode(Arena, Node);
    VNode->Parent = DebugTree->Current;

    if(!DebugTree->Root)
    {
      DebugTree->Root    = VNode;
      DebugTree->Current = VNode;
    }
    else
    {
      if(DebugTree->Current->First == &nil_v_node)
      {
        DebugTree->Current->First = VNode;
        DebugTree->Current->Last  = VNode;
      }
      else
      {
        DebugTree->Current->Last->NextVNode = VNode;
        DebugTree->Current->Last            = VNode;
      }
      DebugTree->Current = VNode;
    }

    for(syntax_node *Sibling = Node->NextNode;
    Sibling && Sibling != &nil_syntax_node;
    Sibling = Sibling->NextNode)
    {
      v_node *SVNode                      = CreateVNode(Arena, Sibling);
      SVNode->Parent                      = DebugTree->Current->Parent;
      DebugTree->Current->Last->NextVNode = SVNode;
      DebugTree->Current->Last            = SVNode;
    }
  }

  return DebugTree;
}

internal void
DebugTree(app_offscreen_buffer *Buffer, debug_tree *DT)
{
  f32 X        = 50.f;
  f32 Y        = 50.f;
  f32 NodeSize = 40.f;
  f32 Padding  = 10.f;

  for(v_node *VNode = DT->Root; VNode && VNode != &nil_v_node; VNode = VNode->NextVNode)
  {
    rect Dest = RectFromSize(V2(X, Y), V2(NodeSize, NodeSize));
    DrawRect(Dest, Color_Yellow, 8.f, 0.f, 1.f);
    X += NodeSize + Padding;
  }
}
