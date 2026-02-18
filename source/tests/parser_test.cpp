#define BASE_NO_ENTRYPOINT 1
#include "base/base.h"
#include "base/base.c"
#include "editor/editor_platform.h"
#include "editor/editor_font.h"
#include "editor/editor_random.h"
#include "editor/editor_math.h"
#include "editor/editor_libs.h"
#include "editor/editor_gl.h"
#include "editor/editor_app.h"
#include "editor/editor_parser.h"
#include "editor/editor_parser.c"

int
main()
{
    app_state test_app = {0};
    arena    *Arena    = ArenaAlloc();

    int fd = open("test.c", O_RDONLY);
    if (fd < 0)
    {
        return 1;
    }

    char *input_buffer = (char *)ArenaPush(Arena, MB(8), 0);

    ssize_t bytes_read = read(fd, input_buffer, MB(2) - 1);
    close(fd);

    if (bytes_read <= 0)
    {
        return 1;
    }
    input_buffer[bytes_read] = '\0';

    for (s32 i = 0; i < bytes_read; ++i)
    {
        test_app.Text[i] = input_buffer[i];
    }

    test_app.TextCount = bytes_read;

    ConcreteSyntaxTree *tree = Parse(&test_app, Arena, 0);

    printf("%s", tree->Root->Token->Lexeme.Data);

    return 0;
}
