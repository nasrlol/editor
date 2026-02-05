#include <GL/glx.h>
#include <unistd.h>

#include "base/base.h"
#include "base/base_test.h"
#include "editor/editor.h"
#include "editor/editor.c"
#include "linux_window.h"

typedef int b32;

global_variable b32 running = 1;

internal linux_window
linux_window_create(void)
{
    linux_window win = {0};

    /* NOTE(nasr): heh */
    GLint att[] = {GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None};

    win.display = XOpenDisplay(0);
    if (!win.display)
    {
        fatal("XOpenDisplay failed");
    }

    XVisualInfo *vi = glXChooseVisual(win.display, win.screen, att);
    if (!vi)
    {
        fatal("glXChooseVisual failed");
    }

    win.screen = DefaultScreen(win.display);
    win.visual = vi;

    win.colormap = XCreateColormap(
    win.display,
    RootWindow(win.display, win.screen),
    win.visual->visual,
    AllocNone);

    XSetWindowAttributes attrs = {0};
    attrs.colormap             = win.colormap;
    attrs.event_mask           = ExposureMask | KeyPressMask | StructureNotifyMask;

    win.window = XCreateWindow(
    win.display,
    RootWindow(win.display, win.screen),
    0,
    0,
    1920,
    1080,
    0,
    win.visual->depth,
    InputOutput,
    win.visual->visual,
    CWColormap | CWEventMask,
    &attrs);

    XMapWindow(win.display, win.window);

    return win;
}

internal void
linux_window_destroy(linux_window *win)
{
    XDestroyWindow(win->display, win->window);
    XCloseDisplay(win->display);
}

internal void
linux_window_poll_events(Display *display)
{
    while (XPending(display))
    {
        XEvent event;
        XNextEvent(display, &event);

        if (event.type == DestroyNotify)
        {
            running = 0;
        }
    }
}

int
LinuxWindowInit()
{
    linux_window   win = linux_window_create();
    opengl_context gl  = opengl_init(&win);

    for (; running;)
    {
        linux_window_poll_events(win.display);

        glXSwapBuffers(win.display, win.window);
        usleep(1000);
    }

    opengl_destroy(&win, &gl);
    linux_window_destroy(&win);

    return 0;
}
