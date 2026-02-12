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

    int fd = open("source/tests/parser_test.cpp", O_RDONLY);
    if (fd < 0)
    {
        return 1;
    }

    char   *input_buffer = (char *)ArenaPush(Arena, MB(8), 0);
    ssize_t bytes_read   = read(fd, input_buffer, MB(5) - 1);
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

    Token *tokens = Parse(&test_app, Arena, 0);
    for (Token *token = tokens; token != 0; token = token->Next)
    {
        printf("Type=%u Lexeme=\"", (TokenType)token->Type);
        for (s32 i = 0; i < token->Lexeme.Size; ++i)
        {
            printf("%c", token->Lexeme.Data[i]);
        }

        printf("\" Line=%d Column=%d\n", token->Line, token->Column);
    }

    return 0;
}
