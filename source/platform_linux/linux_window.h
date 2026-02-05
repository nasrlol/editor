#ifndef WINDOW_H
#define WINDOW_H

#include "base/base.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>

typedef struct linux_window linux_window;

struct linux_window
{
    Display     *display;
    Window       window;
    Colormap     colormap;
    XVisualInfo *visual;
    int          screen;
};

#endif
