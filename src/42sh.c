#include <assert.h>
#include <string.h>

#include "io/io.h"
#include "shell/shell.h"

#define USAGE_STRING "Usage: 42sh [OPTIONS] [SCRIPT] [ARGUMENTS...]\n"

int main(int argc, char **argv, char **envp)
{
    FILE *src_file = NULL;
    struct io_source source;
    if (argc == 1)
    {
        source.source_type = IO_FILE;
        source.source_stream.file = stdin;
    }
    else if (argc >= 2
             && !(strlen(argv[1]) == 2 && memcmp(argv[1], "-c", 2) == 0))
    {
        src_file = fopen(argv[1], "r");
        if (src_file == NULL)
        {
            fprintf(stderr, "Failed to open file: %s\n", argv[1]);
            fprintf(stderr, USAGE_STRING);
            return RET_COMMAND_LINE_GRAMMAR;
        }
        source.source_type = IO_FILE;
        source.source_stream.file = src_file;
        argv = argv + 1;
        argc -= 1;
    }
    else if (argc >= 3 && strlen(argv[1]) == 2 && memcmp(argv[1], "-c", 2) == 0)
    {
        source.source_type = IO_CHAR_STREAM;
        source.source_stream.char_stream.stream = argv[2];
        source.source_stream.char_stream.size = strlen(argv[2]);
        argv = argv + 3;
        argc -= 3;
    }
    else
    {
        fprintf(stderr, "Invalid arguments\n");
        fprintf(stderr, USAGE_STRING);
        return RET_COMMAND_LINE_GRAMMAR;
    }

    int res = shell_loop(argc, argv, envp, source);

    if (src_file != NULL)
        fclose(src_file);

    return res;
}
