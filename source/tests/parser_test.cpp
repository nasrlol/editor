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
#include "editor/editor_lexer.h"
#include "editor/editor_lexer.c"
#include "editor/editor_parser.h"
#include "editor/editor_parser.c"
#include "editor/editor_tree_visualizer.h"
#include "editor/editor_tree_visualizer.c"

int main(s32 argc, char **argv)
{
    char *filename = "test.c";
    if (argc > 1)
    {
        filename = argv[1];
    }

    arena *Arena = ArenaAlloc(.Size = GB(3));
    app_state test_app = {0};

    struct stat st;
    if (stat(filename, &st) != 0)
    {
        return 1;
    }

    int fd = open(filename, O_RDONLY);
    if (fd < 0)
    {
        return 1;
    }

    u64 max_bytes = st.st_size;
    if (max_bytes > 1023)  
    {
        max_bytes = 1023;
    }

    char *buffer = PushArray(Arena, char, max_bytes);

    s64 bytes_read = read(fd, buffer, max_bytes);
    close(fd);

    if (bytes_read <= 0)
    {
        return 1;
    }

    for (s64 i = 0; i < bytes_read; ++i)
    {
        test_app.Text[i] = (rune)buffer[i];
    }

    test_app.Text[bytes_read] = 0;
    test_app.TextCount        = bytes_read;

    TokenList *list = Lex(&test_app, Arena);
    ConcreteSyntaxTree *tree = Parse(Arena, list);

    tree_visualizer(tree, Arena);

    return 0;
}
