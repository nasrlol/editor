/* date = February 23rd 2026 6:09 pm */

#ifndef EDITOR_UI_H
#define EDITOR_UI_H
//~ Types

enum axis2
{
    Axis2_X,
    Axis2_Y,
    Axis2_Count
};
typedef enum axis2 axis2;

// TODO(luca): Metaprogram
enum ui_box_flag
{
    UI_BoxFlag_None =              (0 << 0),
    UI_BoxFlag_DrawBackground =    (1 << 0),
    UI_BoxFlag_DrawBorders =       (1 << 1),
    UI_BoxFlag_DrawDebugBorder =   (1 << 2),
    UI_BoxFlag_DrawShadow =        (1 << 3),
    UI_BoxFlag_DrawDisplayString = (1 << 4),
    
    UI_BoxFlag_CenterTextHorizontally = (1 << 5),
    UI_BoxFlag_CenterTextVertically =   (1 << 6),
    
    UI_BoxFlag_AnimateColorOnHover = (1 << 7),
    UI_BoxFlag_AnimateColorOnPress = (1 << 8),
};
typedef enum ui_box_flag ui_box_flag;

enum ui_size_kind
{
    UI_SizeKind_Null,
    UI_SizeKind_Pixels,
    UI_SizeKind_TextContent,
    UI_SizeKind_PercentOfParent,
    UI_SizeKind_ChildrenSum,
};
typedef enum ui_size_kind ui_size_kind;

typedef struct ui_size ui_size;
struct ui_size
{
    ui_size_kind Kind;
    f32 Value;
    f32 Strictness;
};

typedef struct ui_key ui_key; 
struct ui_key
{
    u64 U64[1];
}; 

typedef struct ui_box ui_box;
struct ui_box
{
    // Tree links
    ui_box *First;
    ui_box *Last;
    ui_box *Prev;
    ui_box *Next;
    ui_box *Parent;
    
    // Hash links
    ui_box *HashNext;
    ui_box *HashPrev;
    
    // Key and generation info
    ui_key Key;
    u64 LastFrameTouchedIndex;
    
    u32 Flags;
    ui_size SemanticSize[Axis2_Count];
    
    v2 FixedPosition;
    v2 FixedSize;
    rect Rec;
    
    str8 DisplayString;
    
    b32 Hovered;
    b32 Clicked;
    b32 Pressed;
    
    f32 BorderThickness;
    f32 Softness;
    v4 BackgroundColor;
    v4 BorderColor;
    v4 TextColor;
    v4 CornerRadii;
};

//~ Globals
global_variable ui_box *UI_Root = 0;
global_variable ui_box *UI_Current = 0;
global_variable arena *UI_BoxArena = 0;
global_variable font_atlas *UI_Atlas = 0; 
global_variable b32 AppendToParent = false;
global_variable b32 UI_GlobalDebugMode = false;
global_variable app_input *UI_Input = 0;
global_variable ui_box *UI_BoxTable = 0;
global_variable u64 UI_BoxTableSize = 0;

read_only global_variable ui_box _UI_NilBox =
{
    &_UI_NilBox, 
    &_UI_NilBox,
    &_UI_NilBox,
    &_UI_NilBox,
    &_UI_NilBox,
    &_UI_NilBox,
    &_UI_NilBox,
};

global_variable ui_box *UI_NilBox = 0;

#endif //EDITOR_UI_H
