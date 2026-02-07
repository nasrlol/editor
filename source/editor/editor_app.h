/* date = January 27th 2026 7:06 pm */

#ifndef EDITOR_APP_H
#define EDITOR_APP_H

global_variable f32 DefaultHeightPx = 24.0f;
global_variable f32 HeightPx = DefaultHeightPx;

typedef struct app_state app_state;
struct app_state
{
    app_font Font;
    
    u64 TextCount;
    rune Text[1024];
    
    s32 CursorPos;
};

#endif //EDITOR_APP_H
