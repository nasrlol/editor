#include "base/base_core.h"
#include <unistd.h>
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

/** NOTE(nasr): generated tests :( **/

char * get_test_input(const char *filename)
{
    arena *Arena = ArenaAlloc();
    int fd = open(filename, O_RDONLY);
    if(fd < 0)
    {
        return 0;
    }
    size_t buffer_size = MB(1);
    char *buffer = (char *)ArenaPush(Arena, buffer_size, 0);
    ssize_t bytes_read = read(fd, buffer, buffer_size - 1);
    close(fd);
    if(bytes_read > 0)
    {
        buffer[bytes_read] = '\0';
    }
    else
    {
        buffer[0] = '\0';
    }
    return buffer;
}

/* NOTE(nasr): stdio is included in editor_font */
void run()
{
    app_state test_app = {0};
    arena *Arena = ArenaAlloc();

    const char *input_file = "test.c";
    char *input_buffer = get_test_input(input_file);

    if(!input_buffer)
    {
        printf("Error: Could not read file '%s'\n", input_file);
        return;
    }

    // Calculate length of file input
    s32 input_length = 0;
    while(input_buffer[input_length] != '\0')
    {
        input_length++;
    }

    // Copy input to test_app
    for(s32 input_index = 0; input_index < input_length; ++input_index)
    {
        test_app.Text[input_index] = input_buffer[input_index];
    }
    test_app.TextCount = input_length;

    Token *tokens = ParseBuffer(&test_app, Arena);
    for(Token *token = tokens; token != 0; token = token->Next)
    {
        printf("Type=%d ", token->Type);
        printf("Lexeme=\"");
        for(s32 LexemeIndex = 0;
                LexemeIndex < token->Lexeme.Size;
                ++LexemeIndex)
        {
            printf("%c", token->Lexeme.Data[LexemeIndex]);
        }
        printf("\" ");
        printf("Line=%d Column=%d\n", token->Line, token->Column);
    }
}

int main()
{
    run();
    return 0;
}
