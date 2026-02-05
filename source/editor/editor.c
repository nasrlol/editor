#include <GL/glx.h>

#include "base/base.h"
#include "base/base_test.h"
#include "editor.h"

internal opengl_context
opengl_init(linux_window *win)
{
    opengl_context gl = {0};

    gl.glx = glXCreateContext(win->display, win->visual, 0, GL_TRUE);

    if (!(gl.glx))
    {
        fatal("failed to initilizate the glx contentx");
    }

    glXMakeCurrent(win->display, win->window, gl.glx);

    return gl;
}

internal void
opengl_destroy(linux_window *win, opengl_context *gl)
{
    glXMakeCurrent(win->display, None, 0);
    glXDestroyContext(win->display, gl->glx);
}

internal void
draw_text(opengl_context *gl, int x, int y, const char *text)
{
    /** TODO(nasr): use glxlists or something like that */
}
