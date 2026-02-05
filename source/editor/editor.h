#ifndef EDITOR_H
#define EDITOR_H

#include "base/base.h"
#include "platform_linux/linux_window.h"

#include <GL/gl.h>
#include <GL/glx.h>

typedef struct buffer         buffer;
typedef struct opengl_context opengl_context;

struct opengl_context
{
    GLXContext glx;
    GLuint     font_base;
};

struct buffer
{
    char *value;
};

internal opengl_context
opengl_init(linux_window *win);

internal void
opengl_destroy(linux_window *win, opengl_context *gl);

internal void
draw_text(opengl_context *gl, int x, int y, const char *text);

#endif
