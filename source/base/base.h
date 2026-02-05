#ifndef BASE_H
#define BASE_H

#include <stdint.h>
#include <unistd.h>
#include <stddef.h>
#include <string.h>

/* assert an expression and output the file and the line */

#define internal static
#define global_variable static
#define local_persist static

#define ERR_OK 0
#define ERR_IO 1
#define ERR_PARSE 2
#define ERR_PERM 3
#define ERR_INVALID 4

#define KiB(n) (((u64)(n)) << 10)
#define MiB(n) (((u64)(n)) << 20)
#define GiB(n) (((u64)(n)) << 30)

#define unused(x) (void)(x)

#define PATH_MAX_LEN 128
#define BUFF_SMALL 128
#define BUFF_DEFAULT 256
#define BUFF_LARGE 512

#define NIL 0

#define DEPRECATED __attribute__((__deprecated__))

#if defined(__arm__) || defined(__aarch64__)
/* NOTE(nasr): acts as a crash the program when using a gdb */
#define breakpoint __asm__ volatile("brk #0");
#define temp_breakpoint __asm__ volatile("udf #0");
#elif defined(__i386__) || defined(__x86_64__)
#define breakpoint __asm__ volatile("int3");
#endif

#define MemCpy(dest, src, len) memcpy((dest), (src), (len))
#define MemSet(dest, len) memset((dest), (0), (len))

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t  u8;

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef float  f32;
typedef double f64;

typedef i32 b32;
typedef i16 b16;
typedef u8  b8;

typedef uintptr_t umm;
typedef intptr_t  smm;

#define TRUE (0 == 0)
#define FALSE (0 != 0)

typedef struct s8 s8;

struct s8
{
    char *value;
    umm   size;
};

#endif
