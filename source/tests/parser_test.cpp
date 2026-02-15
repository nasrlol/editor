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

void
PrintTree(SyntaxNode *Node)
{
	if (!Node)
	{
		return;
	}

	if (Node->Token)
	{
		if (Node->Token->Type != 0)
		{
			printf(S8Fmt " -- Type: %d\n", S8Arg(Node->Token->Lexeme), (TokenType)Node->Token->Type);
		}

		if (Node->Child)
		{
			for (int NodeIndex = 0;
                    Node->Child[NodeIndex] != NULL;
                    ++NodeIndex)
			{
				PrintTree(Node->Child[NodeIndex]);
			}
		}

		if (Node->NextNode)
        {
			PrintTree(Node->NextNode);
		}
	}
}

int
main(s32 argc, char **argv)
{
	char *filename = "test.c";
	if (argc > 1)
	{
		filename = argv[1];
	}

	app_state test_app = {0};
	arena	 *Arena	   = ArenaAlloc();

	int fd = open(filename, O_RDONLY);
	if (fd < 0)
		return 1;

	umm bytes_read = read(fd, test_app.Text, 1024);
	close(fd);

	test_app.TextCount = bytes_read;
	if (bytes_read < 1024)
	{
		test_app.Text[bytes_read] = '\0';
	}

	TokenList		   *list = Lex(&test_app, Arena);
	ConcreteSyntaxTree *tree = Parse(Arena, list);

	if (tree && tree->Root)
	{
		PrintTree(tree->Root);
	}

	return 0;
}
