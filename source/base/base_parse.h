#ifndef BASE_PARSE_H
#define BASE_PARSE_H

#include "base.h"
#include "base_arena.h"

#define COMPARE_STRING(c1, c2) compare_string((char *)c1, (char *)c2)

typedef struct proc_entry proc_entry;
typedef struct proc_file  proc_file;

struct proc_file
{
    i32         count;
    proc_entry *entries;
};

struct proc_entry
{
    char value[16];
    char key[16];
};

typedef struct
{
    i8 *start;
    i8 *end;
    umm len;
} Line;

internal b8
compare_string(char *c1, char *c2);

internal proc_file *
parse_proc_files(char *path, mem_arena *arena);

internal u64
parse_u64(char *buf, umm len);

internal b8
is_numeric(char *s);

#endif
