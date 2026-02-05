#define debug
#include "base_parse.h"
#undef debug

#include <fcntl.h>
#include <unistd.h>

/*
 * TODO(nasr): to fix oops
 * */
internal b8
compare_string(char *c1, char *c2)
{
    for (u64 word_idx = 0; word_idx <= sizeof(*c1); ++word_idx)
    {
        if (*c1 != *c2)
        {
            return -1;
        }
        ++c1;
        ++c2;
    }

    return 0;
}

/**
 * Helper function to parse strings to int using ascii codes
 * */
internal u64
parse_u64(char *buf, umm len)
{
    u64 value = 0;

    for (umm buffer_idx = 0; buffer_idx < len; ++buffer_idx)
    {
        char c = buf[buffer_idx];
        if (c < '0' || c > '9')
        {
            break;
        }
        value = value * 10 + (c - '0');
    }

    return value;
}

/*
 * is_numeric - Check if a string contains only digits
 * @s: String to check
 *
 * Return: 1 if string contains only numeric characters, 0 otherwise
 */
internal b8
is_numeric(char *s)
{
    for (; *s; ++s)
    {
        if (*s < '0' || *s > '9')
        {
            return 0;
        }
    }
    return 1;
}

/*
 * TODO(nasr): checkout i think there is a buffer overflow happening somewhere
 * */
internal proc_file *
parse_proc_files(char *path, mem_arena *arena)
{
    if (!path || !arena)
    {
        return NULL;
    }

    i32 fd = open(path, O_RDONLY);
    if (fd < 0)
    {
        return NULL;
    }

    char *buffer = PushArray(arena, char, KiB(4));
    u64   bytes  = read(fd, buffer, KiB(4));
    close(fd);

    if (bytes == 0)
    {
        return NULL;
    }

    /* guessing the count to 256 because i dont want to do a double pass of the buffer */
    proc_file *pf = PushStruct(arena, proc_file);
    pf->entries   = PushArray(arena, proc_entry, 256);

    u64 line_start  = 0;
    u64 delim       = -1;
    u64 entry_index = 0;

    for (u64 index = 0; index < bytes; ++index)
    {
        if (buffer[index] == ':' && delim == (u64)-1)
        {
            delim = index;
        }
        else if (buffer[index] == '\n')
        {
            if (delim != (-1))
            {
                u64 key_len = delim - line_start;
                if (key_len >= sizeof(pf->entries[entry_index].key))
                {
                    key_len = sizeof(pf->entries[entry_index].key) - 1;
                }

                u64 val_start = delim + 1;
                while (val_start < index && (buffer[val_start] == ' ' || buffer[val_start] == '\t'))
                {
                    val_start++;
                }

                u64 val_len = index - val_start;
                if (val_len >= sizeof(pf->entries[entry_index].value))
                {
                    val_len = sizeof(pf->entries[entry_index].value) - 1;
                }

                MemCpy(pf->entries[entry_index].key, buffer + line_start, key_len - 1);
                MemCpy(pf->entries[entry_index].value, buffer + val_start, val_len);

                pf->entries[entry_index].key[key_len]   = '\0';
                pf->entries[entry_index].value[val_len] = '\0';

                ++pf->count;
                ++entry_index;
            }

            line_start = index + 1;
            delim      = (u64)-1;
        }
    }

    return (pf);
}
