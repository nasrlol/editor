#version 330 core

#define v2 vec2
#define v3 vec3
#define v4 vec4 
#define f32 float

layout (location = 0) in v2 pos;
layout (location = 1) in v4 dest_rect;

out v2 LocalPos;
flat out v4 DestRect;

uniform vec2 Viewport;

void main()
{
    LocalPos = pos;
    DestRect = dest_rect;
    
    v2 PosClip = v2(2.0*(pos.x/Viewport.x) - 1.0,
                    2.0*(1.0 - pos.y/Viewport.y) - 1.0);
    
    gl_Position = v4(PosClip, -1.0, 1.0);
}