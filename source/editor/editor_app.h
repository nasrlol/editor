/* date = January 27th 2026 7:06 pm */

#ifndef EDITOR_APP_H
#define EDITOR_APP_H

#define DefaultHeightPx 24

typedef struct font_atlas font_atlas;
struct font_atlas
{
    u32 Width;
    u32 Height;
    u8 *Data;
    
    rune FirstCodepoint;
    u32 CodepointsCount;
    
    f32 PixelScaleWidth;
    f32 PixelScaleHeight;
    
    stbtt_packedchar *PackedChars;
    stbtt_aligned_quad *AlignedQuads;
};

typedef struct app_state app_state;
struct app_state
{
    app_font Font;
    font_atlas FontAtlas;
    arena *FontAtlasArena; 
    f32 PreviousHeightPx;
    f32 HeightPx;
    
    u64 TextCount;
    rune Text[1024];
    
    s32 CursorPos;
};

#endif //EDITOR_APP_H
