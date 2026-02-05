#include "base_test.h"
#include "base_arena.h"

#include <stdint.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "base_mem.h"
#include "base.h"

internal mem_arena *
arena_create(u64 capacity)
{
    mem_arena *arena = (mem_arena *)mmap(
    /* kernel decides where to throw the arena */
    NULL,
    capacity + sizeof(mem_arena),
    PROT_READ | PROT_WRITE,
    MAP_SHARED | MAP_ANONYMOUS,
    -1,
    0);

    if (arena == MAP_FAILED)
    {
        return NULL;
    }

    arena->capacity          = capacity;
    arena->base_position     = (u8 *)arena + sizeof(mem_arena);
    arena->current_position  = 0;
    arena->previous_position = 0;

    return arena;
}

internal void
arena_destroy(mem_arena *arena)
{
    if (!arena)
    {
        return;
    }

    int code = munmap(arena, arena->capacity + sizeof(mem_arena));
}

internal void *
arena_alloc(mem_arena *arena, u64 size)
{
    if (!arena)
    {
        return NULL;
    }

    u64 aligned = Align(arena->current_position, ARENA_ALIGN);
    u64 new_pos = aligned + size;

    if (new_pos > arena->capacity)
    {
        return NULL;
    }

    void *out = arena->base_position + aligned;

    arena->previous_position = arena->current_position;
    arena->current_position  = aligned + size;

    MemSet(out, size);

    return out;
}

/////////////////////////////////////////////////////////////////////////////
internal void
arena_pop(mem_arena *arena, u64 size)
{
    size = MIN(size, arena->current_position);
    arena->current_position -= size;
}

internal void
arena_pop_to(mem_arena *arena, u64 pos)
{
    u64 size = pos < arena->current_position ? arena->current_position - pos : 0;
    arena_pop(arena, size);
}

internal void
arena_clear(mem_arena *arena)
{
    arena->current_position = 0;
}
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
internal mem_arena *
arena_resize_align(mem_arena *arena, void *old_memory, u64 new_size, u64 old_size, umm alignment)
{
    u8 *old_mem = (u8 *)old_memory;

    if (!is_pow(alignment))
    {
        Align(arena->current_position, alignment);
    }

    if (old_memory == NULL || old_size == 0)
    {
        return (mem_arena *)arena_alloc(arena, new_size);
    }
    else if ((old_mem >= arena->base_position && old_mem < arena->base_position + arena->capacity))
    {
        if ((arena->base_position + arena->previous_position) == old_memory)
        {
            arena->current_position = arena->previous_position + new_size;
            if (new_size > old_size)
            {
                MemSet(&arena->current_position, new_size - old_size);
            }
            return (mem_arena *)old_memory;
        }
        else
        {
            void *new_memory = arena_alloc(arena, new_size);
            umm   copy_size  = old_size < new_size ? old_size : new_size;
            memmove(new_memory, old_mem, copy_size);
        }
    }
    else
    {
        check(0);
    }
    return NULL;
}

internal mem_arena *
arena_resize(mem_arena *arena, void *old_memory, u64 new_size, u64 old_size)
{
    return arena_resize_align(arena, old_memory, new_size, old_size, ARENA_ALIGN);
}
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
internal temp_arena
temp_arena_begin(mem_arena *arena)
{
    temp_arena t;
    t.arena          = arena;
    t.start_position = arena->current_position;

    return t;
}

internal void
temp_arena_end(temp_arena temp)
{
    temp.arena->current_position = temp.start_position;
}
////////////////////////////////////////////////////////////////////////////
