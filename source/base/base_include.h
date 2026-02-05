#ifndef BASE_INCLUDE_H
#define BASE_INCLUDE_H

#include "base.h"
#include "base_mem.h"
#include "base_arena.h"
#include "base_parse.h"
#include "base_stack.h"
#include "base_test.h"

#ifdef BASE_UNITY

#include "base_mem.c"
#include "base_arena.c"
#include "base_parse.c"
#include "base_stack.c"
#include "base_test.c"

#endif

#endif
