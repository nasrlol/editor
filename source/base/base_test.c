#include <unistd.h>
#include "base_test.h"

internal void
fatal(char *msg)
{
    write(STDERR_FILENO, msg, STR_LEN(msg));
    write(STDERR_FILENO, "\n", 1);
    _exit(1);
}
