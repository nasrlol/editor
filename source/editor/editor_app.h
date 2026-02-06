/* date = January 27th 2026 7:06 pm */

#ifndef EDITOR_APP_H
#define EDITOR_APP_H

struct app_state
{
    app_font Font;
    
    u64 TextCount;
    rune Text[1024];
    
    s32 CursorLinePos;
};

#endif //EDITOR_APP_H
