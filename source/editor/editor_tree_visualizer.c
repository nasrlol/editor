 // We use a global or static pointer to keep track of the second window
static P_context VisualizerWindow = 0;
static b32 VisualizerRunning = false;
static app_offscreen_buffer VisualizerBuffer = {};

internal void
tree_visualizer(ConcreteSyntaxTree *tree, arena *Arena)
{
    // 1. If the window doesn't exist, initialize it
    if(!VisualizerWindow)
    {
        VisualizerBuffer.Width = 800;
        VisualizerBuffer.Height = 600;
        VisualizerBuffer.BytesPerPixel = 4;
        // Allocate pixels from the Arena
        VisualizerBuffer.Pixels = PushArray(Arena, u8, 
            VisualizerBuffer.Width * VisualizerBuffer.Height * VisualizerBuffer.BytesPerPixel);
        
        VisualizerRunning = true;
        VisualizerWindow = P_ContextInit(Arena, &VisualizerBuffer, &VisualizerRunning);
    }

    // 2. Process events for the second window specifically
    if(VisualizerRunning)
    {
        app_input DummyInput = {}; 
        P_ProcessMessages(VisualizerWindow, &DummyInput, &VisualizerBuffer, &VisualizerRunning);

        // 3. Switch OpenGL focus to this window
        linux_x11_context *Ctx = (linux_x11_context *)VisualizerWindow;
        glXMakeCurrent(Ctx->DisplayHandle, Ctx->WindowHandle, Ctx->OpenGLContext);

        // --- DRAWING CODE START ---
        // Clear screen to a different color (dark grey)
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // TODO: Call your actual tree drawing logic here
        
        // --- DRAWING CODE END ---

        // 4. Swap and return context to the main window
        glXSwapBuffers(Ctx->DisplayHandle, Ctx->WindowHandle);
        
        // IMPORTANT: You must swap back to the main window's context 
        // after this function ends, or your main loop will draw to the wrong window!
    }
}
