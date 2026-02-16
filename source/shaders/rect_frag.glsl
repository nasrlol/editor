#version 330 core

#define v2 vec2
#define v3 vec3
#define v4 vec4 
#define f32 float

in v2  VS_Pos;
in v4  VS_Dest;
in v4  VS_Color;
in f32 VS_CornerRadius;
in f32 VS_BorderThickness;
in f32 VS_Softness;

uniform v2 Viewport;

out v4 FragColor;

f32
BoxSDF(v2 Pos, v2 HalfDim, f32 CornerRadius)
{
    v2 q = abs(Pos) - (HalfDim - CornerRadius);
    
    f32 Distance = min(max(q.x, q.y), 0.0) + length(max(q, 0.0)) - CornerRadius;
    
    return Distance;
}

void main()
{
    v4 Color = VS_Color;
    
    // TODO(luca): This should happen in the vertex shader.
    v2 Min = VS_Dest.xy;
    v2 Max = VS_Dest.zw;
    v2 HalfSize = (Max - Min)/2.0;
    v2 Center = Min + HalfSize;
    v2 Pos = VS_Pos - Center;
    
    f32 tBorder = 1.0;
    if(VS_BorderThickness > 0)
    {
        f32 sBorder = BoxSDF(Pos,
                             HalfSize - v2(VS_Softness*2.0, VS_Softness*2.0) - VS_BorderThickness,
                             max(VS_CornerRadius-VS_BorderThickness, 0.0));
        tBorder = smoothstep(0.0, 2.0*VS_Softness, sBorder);
    }
    if(tBorder < 0.001f)
    {
        discard;
    }
    
    f32 tCorner = 1;
    if(VS_CornerRadius > 0.0 || VS_Softness > 0.75f)
    {
        f32 sCorner = BoxSDF(Pos,
                             HalfSize - v2(VS_Softness*2.0, VS_Softness*2.0),
                             VS_CornerRadius);
        tCorner = 1-smoothstep(0.0, 2.0*VS_Softness, sCorner);
    }
    
    Color.a *= tBorder;
    Color.a *= tCorner;
    
    FragColor = Color;
}