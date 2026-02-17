 // We use a global or static pointer to keep track of the second window
static P_context VisualizerWindow = 0;
static b32 VisualizerRunning = false;
static app_offscreen_buffer VisualizerBuffer = {};

internal void
tree_visualizer(ConcreteSyntaxTree *tree, arena *Arena)
{
    if(!VisualizerWindow)
    {
        VisualizerBuffer.Width = 800;
        VisualizerBuffer.Height = 600;
        VisualizerBuffer.BytesPerPixel = 4;
        VisualizerBuffer.Pixels = PushArray(Arena, u8, 
            VisualizerBuffer.Width * VisualizerBuffer.Height * VisualizerBuffer.BytesPerPixel);
        
        VisualizerRunning = true;
        VisualizerWindow = P_ContextInit(Arena, &VisualizerBuffer, &VisualizerRunning);
    }

    if(VisualizerRunning)
    {
        app_input DummyInput = {}; 
        P_ProcessMessages(VisualizerWindow, &DummyInput, &VisualizerBuffer, &VisualizerRunning);

        P_context context = P_ContextInit(Arena, &VisualizerBuffer, &VisualizerRunning);
    }
}
