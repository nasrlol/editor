/* date = January 27th 2026 7:06 pm */

#ifndef EDITOR_APP_H
#define EDITOR_APP_H

typedef struct app_state app_state;
typedef struct cursor   cursor;

global_variable f32 HeightPx = 24.0f;

struct app_state
{
    app_font Font;
    
    u64 TextCount;
    rune Text[1024];
    
    s32 CursorPos;
};

struct cursor
{
    s32 XOffset;
    s32 YOffset;
};

#endif //EDITOR_APP_H
