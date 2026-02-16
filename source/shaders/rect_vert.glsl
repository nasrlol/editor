#version 330 core

#define v2 vec2
#define v3 vec3
#define v4 vec4 
#define f32 float

layout (location = 0) in v2 P_Pos;
layout (location = 1) in v4 I_Dest;
layout (location = 2) in v4  I_Color;
layout (location = 3) in f32 I_CornerRadius;
layout (location = 4) in f32 I_BorderThickness;
layout (location = 5) in f32 I_Softness;

uniform v2 Viewport;

out v2  VS_Pos;
out v4  VS_Dest;
out v4  VS_Color;
out f32 VS_CornerRadius;
out f32 VS_BorderThickness;
out f32 VS_Softness;

void main()
{
    VS_Pos = P_Pos;
    VS_Dest = I_Dest;
    VS_Color = I_Color;
    VS_CornerRadius = I_CornerRadius;
    VS_BorderThickness = I_BorderThickness;
    VS_Softness = I_Softness;
    
    v2 NormalPos = 2.0*v2((      VS_Dest.x/Viewport.x),
                          (1.0 - VS_Dest.y/Viewport.y)) - 1.0;
    
    gl_Position = v4(NormalPos, -1.0, 1.0);
}