/* date = January 27th 2026 7:06 pm */

#ifndef EX_APP_H
#define EX_APP_H

struct entity
{
    v2 P;
    
    b32 IsDead;
};

struct app_state
{
    arena *EntitiesArena;
    u32 EntitiesCount;
    entity *Entities;
    u32 EnemiesCount;
    entity *Enemies;
    
    entity *Player;
    f32 PlayerdP;
    
    u32 EnemiesPerRow;
    
    u32 EnemiesDeadCount;
};

#endif //EX_APP_H
