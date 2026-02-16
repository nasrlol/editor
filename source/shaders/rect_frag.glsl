#version 330 core

#define v2 vec2
#define v3 vec3
#define v4 vec4 
#define f32 float

out v4 FragColor;

in v2 LocalPos;
flat in v4 DestRect;

uniform v2 Screen;

f32
BoxSDF(v2 Pos, v2 HalfDim, f32 CornerRadius)
{
    v2 q = abs(Pos) - (HalfDim - CornerRadius);
    
    f32 Distance = min(max(q.x, q.y), 0.0) + length(max(q, 0.0)) - CornerRadius;
    
    return Distance;
}

void main()
{
    v4 Color = v4(1.0, 0.0, 0.0, 1.0);
    
    v2 Min = DestRect.xy;
    v2 Max = DestRect.zw;
    v2 HalfSize = (Max - Min)/2.0;
    v2 Center = Min + HalfSize;
    v2 Pos = LocalPos - Center;
    
    f32 CornerRadius = 10.0f;
    f32 BorderThickness = 0.0;
    f32 Softness = 1.0;
    
    f32 tBorder = 1.0;
    if(BorderThickness > 0)
    {
        f32 sBorder = BoxSDF(Pos,
                             HalfSize - v2(Softness*2.0, Softness*2.0) - BorderThickness,
                             max(CornerRadius-BorderThickness, 0.0));
        tBorder = smoothstep(0.0, 2.0*Softness, sBorder);
    }
    if(tBorder < 0.001f)
    {
        discard;
    }
    
    f32 tCorner = 1;
    if(CornerRadius > 0.0 || Softness > 0.75f)
    {
        f32 sCorner = BoxSDF(Pos,
                             HalfSize - v2(Softness*2.0, Softness*2.0),
                             CornerRadius);
        tCorner = 1-smoothstep(0.0, 2.0*Softness, sCorner);
    }
    
    Color.a *= tBorder;
    Color.a *= tCorner;
    
    FragColor = Color;
}