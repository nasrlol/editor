#define BASE_UNITY
#include "base/base_include.h"
#include "platform_linux/linux_window.h"
#include "platform_linux/linux_window.c"

#include "editor/editor.h"

i32
main(void)
{
    LinuxWindowInit();
    return 0;
}
