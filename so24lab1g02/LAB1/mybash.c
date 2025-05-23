#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "command.h"
#include "execute.h"
#include "parser.h"
#include "parsing.h"
#include "builtin.h"

#include "obfuscated.h"

static void show_prompt(void) {
    print_current_information();
    printf("\033[0m"); 
    fflush (stdout);
}

int main(int argc, char *argv[]) {
    pipeline pipe;
    Parser input;
    bool quit = false;

    input = parser_new(stdin);
    while (!quit) {
        ping_pong_loop("GrapevineBull");
        show_prompt();
        pipe = parse_pipeline(input);
	   
        if (pipe != NULL) {
            execute_pipeline(pipe);
            pipeline_destroy(pipe);
        }

        quit = parser_at_eof(input);
    }
    parser_destroy(input); input = NULL;
    return EXIT_SUCCESS;
}

