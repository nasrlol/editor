#version 330 core

#define v2 vec2
#define v3 vec3
#define v4 vec4 
#define f32 float

// TODO(luca): Metaprogram location offsets.
layout (location = 0) in v2  P_Pos;
layout (location = 1) in v4  I_Dest;
layout (location = 2) in v4  I_Color0;
layout (location = 3) in v4  I_Color1;
layout (location = 4) in v4  I_Color2;
layout (location = 5) in v4  I_Color3;
layout (location = 6) in v4  I_CornerRadii;
layout (location = 7) in f32 I_BorderThickness;
layout (location = 8) in f32 I_Softness;

uniform v2 Viewport;

out v2  VS_Pos;
out v4  VS_Dest;
out v4  VS_Color;
out f32 VS_CornerRadius;
out f32 VS_BorderThickness;
out f32 VS_Softness;

void main()
{
    v4 Colors[] = v4[](I_Color0, I_Color1, I_Color2, I_Color3);
    
    VS_Dest = I_Dest;
    
    VS_Color = Colors[gl_VertexID];
    VS_CornerRadius = I_CornerRadii[gl_VertexID];
    
    VS_BorderThickness = I_BorderThickness;
    VS_Softness = I_Softness;
    
    v2 Min = I_Dest.xy;
    v2 Max = I_Dest.zw;
    
    v2 PosInQuad = (P_Pos + 1.0) * 0.5;
    v2 PixelPos = mix(Min, Max, PosInQuad);
    
    VS_Pos = PixelPos;
    
    v2 ClipPos = (PixelPos / Viewport) * 2.0 - 1.0;
    ClipPos.y = -ClipPos.y;
    
    gl_Position = v4(ClipPos, 0.0, 1.0);
}