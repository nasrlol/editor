s32 AttributesOffsets[] =
{
4,
4,
4,
4,
4,
4,
4,
1,
1,
1,
};
enum ui_box_flag
{
UI_BoxFlag_None                   = (0 << 0),
UI_BoxFlag_DrawBackground         = (1 << 1),
UI_BoxFlag_DrawBorders            = (1 << 2),
UI_BoxFlag_DrawDebugBorder        = (1 << 3),
UI_BoxFlag_DrawShadow             = (1 << 4),
UI_BoxFlag_DrawDisplayString      = (1 << 5),
UI_BoxFlag_CenterTextHorizontally = (1 << 6),
UI_BoxFlag_CenterTextVertically   = (1 << 7),
UI_BoxFlag_MouseClickability      = (1 << 8),
UI_BoxFlag_Clip                   = (1 << 9),
UI_BoxFlag_TextWrap               = (1 << 10),
UI_BoxFlag_DrawCursor             = (1 << 11),
};
typedef enum ui_box_flag ui_box_flag;
