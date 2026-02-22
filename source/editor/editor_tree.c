internal void
visualize_tree(concrete_syntax_tree *Tree, arena *Arena, app_offscreen_buffer *Buffer, app_input *Input)
{
 if (!Tree || !Tree->Root)
 {
  Assert(0);
  return;
 }

 f32 XOffset = 100.f;
 f32 YOffset = 100.f;
 f32 NodeW   = 50.f;
 f32 NodeH   = 50.f;
 f32 PadX    = 60.f;
 f32 PadY    = 60.f;

 // depth-first traversal using Parent pointer to backtrack
 syntax_node *Node = Tree->Root;
 while (Node) { v4 NodeColor = Color_Blue;
  if (Node->Token && Node->Token->Type)
  {
   switch (Node->Token->Type)
   {
     // coole tree syntax highlighting
    case (TokenIdentifier):
    {
     NodeColor = Color_Green;
     break;
    }
    case (TokenIdentifierValue):
    {
     NodeColor = Color_Yellow;
    }
    break;
    case (TokenNumber):
    {
     NodeColor = Color_Orange;
    }
    break;
    case (TokenFunc):
    {
     NodeColor = Color_Red;
    }
    break;
    default:
    {
     NodeColor = Color_Blue;
    }
    break;
   }
  }

  rect           Dest = RectFromSize(V2(XOffset, YOffset), V2(NodeW, NodeH));
  rect_instance *Inst = DrawRect(Dest, NodeColor, 0.f, 1.f, 1.f);

  Inst->Color0.W = 1.f;
  Inst->Color1.W = 1.f;
  Inst->Color2.W = 1.f;
  Inst->Color3.W = 1.f;

  if (!Node)
  {
   Log("No more nodes");
   return;
  }
  // based on luca's tree visualization
  // children on the right increasing
  else if (Node->First)
  {
   XOffset += PadX;
   Node = Node->First;
  }
  else if (Node->NextNode)
  {
   YOffset += PadY;
   Node = Node->NextNode;
  }
  else
  {
   while (Node->Parent && !Node->Parent->NextNode)
   {
    Node = Node->Parent;
    XOffset -= PadX;
   }

   if (Node->Parent)
   {
    Node = Node->Parent->NextNode;
    XOffset -= PadX;
    YOffset += PadY;
   }
   else
   {
    break;
   }
  }
 }
}
