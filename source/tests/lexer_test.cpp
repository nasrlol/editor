#include "base/base_os.h"
#include "editor/editor_lexer.h"
#include "editor/editor_lexer.c"

internal void TestParseBuffer()
{
    app_state test_app = {0};

    char test_input[] = "int main() { return 0; }";
    test_app.Text = test_input;
    test_app.TextCount = sizeof(test_input) - 1;
    str8 *tokens = ParseBuffer(&test_app);
}


int main()
{

    TestParseBuffer();
}
