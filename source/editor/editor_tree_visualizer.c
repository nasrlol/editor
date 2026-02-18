internal void
tree_visualizer(ConcreteSyntaxTree *tree, arena *Arena, app_offscreen_buffer *Buffer, app_memory *Memory)
{
	v2	NodePosition;
	f32 XOffset;

	if (!tree)
	{
		Assert(0);
	}

	for (; tree->Current;)
	{
		gl_handle node_shader;

		if (tree->Current->NextNode)
		{
			tree->Current = tree->Current->NextNode;

			node_shader	  = gl_ProgramFromShaders(
                    Arena,
                    Memory->ExeDirPath, 
                    S8("../source/shaders/rect_vert.glsl"),
                    S8("../source/shaders/rect_rect.glsl"));

            visual_node *vs = PushStruct(Arena, visual_node);

			if (tree->Current->Token->Flags & FlagDirty)
			{
				// TODO(nasr): draw a special kind of node
			}
			else
			{
				// NOTE(nasr): we dont know what this does yet
				v4 Dest = V4(
                        vs->pos.X,
                        vs->pos.Y,
                        vs->pos.X + 100.f,
                        vs->pos.Y + 40.f);

                v4 NodeColor;
                if (tree->Current->Token->Type == TokenIdentifier)
                {
                    NodeColor = Color_Green;
                }

                DrawRect(Dest, NodeColor, 5.f, 1.f, 0.5f);
				// draw inside the node
				// make sure the node is is big enough
				// position inside the node
			}
		}
		// 1) recursive iteration
		// 2) manual iteration

		{
			// v4 rect position;
		}
	}
}
