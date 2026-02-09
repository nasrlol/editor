internal arena *
ArenaAlloc_(arena_alloc_params Params)
{
    arena *Arena = 0;
    
    u64 Size = Params.DefaultSize;
    if(Params.Size)
    {
        Size = Params.Size;
    }
    
    void *Base = OS_Allocate(Size);
    AsanPoisonMemoryRegion(Base, Size);
    
    u64 HeaderSize = sizeof(arena);
    AsanUnpoisonMemoryRegion(Base, HeaderSize);
    
    Arena = (arena *)Base;
    Arena->Base = Base;
    Arena->Pos = HeaderSize;
    Arena->Size = Size;
    
    return Arena;
}

internal arena *
PushArena(arena *Arena, u64 Size)
{
    arena *Result = 0;
    
    Result = PushStruct(Arena, arena);
    Result->Size = Size;
    Result->Base = ArenaPush(Arena, Result->Size, false);
    Result->Pos = 0;
    AsanPoisonMemoryRegion(Result->Base, Result->Size);
    
    return Result;
}

internal void *
ArenaPush(arena *Arena, u64 Size, b32 Zero)
{
    void *Result = (u8 *)Arena->Base + Arena->Pos;
    
    AsanUnpoisonMemoryRegion(Result, Size);
    if(Zero)
    {        
        MemorySet(Result, 0, Size);
    }
    
    Assert(Arena->Pos + Size < Arena->Size);
    Arena->Pos += Size;
    
    return Result;
}

internal void *
ArenaPushAligned(arena *Arena, u64 Size, u64 Alignment, b32 Zero)
{
    void *Result = 0;
    
    u8 *Base = (u8 *)Arena->Base + Arena->Pos;
    u64 BaseAddress = (u64)Base;
    
    u64 Leftover = (BaseAddress % Alignment);
    Arena->Pos += (Alignment - Leftover);
    
    Result = ArenaPush(Arena, Size, Zero);
    
    return Result;
}

internal u64 
PadSize(u64 Size, u64 Padding)
{
    u64 Result = Size;
    
    u64 Leftover = Size%Padding;
    Result += (Padding - Leftover);
    Assert(Result % Padding == 0);
    
    return Result;
}

internal u64 
BeginScratch(arena *Arena)
{
    u64 Result = Arena->Pos;
    return Result;
}

internal void 
EndScratch(arena *Arena, u64 BackPos)
{
    Arena->Pos = BackPos;
    
    AsanPoisonMemoryRegion((u8 *)Arena->Base + Arena->Pos,
                           Arena->Size - Arena->Pos);
}
