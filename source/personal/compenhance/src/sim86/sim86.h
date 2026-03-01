/* date = August 13th 2025 1:51 pm */

#ifndef SIM86_H
#define SIM86_H

#define Assert(Expression) if(!(Expression)) { __asm__ volatile("int3"); }
#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

#endif //SIM86_H
