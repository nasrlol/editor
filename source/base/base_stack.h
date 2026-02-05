#ifndef STACK_H
#define STACK_H

#include "base.h"

typedef struct mem_stack        mem_stack;
typedef struct mem_stack_header mem_stack_header;

struct mem_stack
{
    u8 *base_position;
    umm current_offset;
    umm capacity;
};

struct mem_stack_header
{
    u8 padding;
    u8 previous_offset;
};

internal mem_stack *
stack_create(u64 capacity);

internal umm
calculate_padding(umm pointer, umm alignment, umm header_size);

internal mem_stack *
stack_push_align(mem_stack *stack, u64 size, umm alignment);

internal void *
stack_push(mem_stack *stack, umm size);

internal void
stack_pop(mem_stack *stack, void *pointer);

internal mem_stack *
stack_resize_align(mem_stack *stack, void *pointer, u64 old_size, u64 new_size, u64 alignment);

internal void
stack_pop_all(mem_stack *stack);

internal void
stack_destroy(mem_stack *stack);
#endif
