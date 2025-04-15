#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include "parsing.h"
#include "parser.h"
#include "command.h"

static scommand parse_scommand(Parser p) {
    arg_kind_t kind;
    char* par = NULL;
    scommand scom = scommand_new();
    
    while (!parser_at_eof(p)) {
        par = parser_next_argument (p, &kind); 
    
        if ( par != NULL) {
            if (kind == ARG_NORMAL) {
            scommand_push_back (scom, par);
            }
            if (kind == ARG_INPUT) {
            scommand_set_redir_in(scom, par);
            }
            if (kind == ARG_OUTPUT) {
            scommand_set_redir_out(scom, par);
            }
        }else if (kind == ARG_INPUT || kind == ARG_OUTPUT) {
            return NULL;
        } else {
            break;
        }
    }
    return scom;
}


pipeline parse_pipeline(Parser p) {
    assert(p != NULL && !parser_at_eof (p));
    pipeline result = pipeline_new();
    scommand cmd = NULL;
    bool error = false, another_pipe=true;
    
    while (another_pipe) {
        cmd = parse_scommand(p);
        error = (cmd == NULL);
        if (error) {
            printf("error cmd\n");
            pipeline_destroy(result);
            result = NULL;
            return result;
        }
        parser_skip_blanks(p);
        pipeline_push_back(result , cmd);
        parser_op_pipe(p , &another_pipe);
    }
    bool wait , gar;
    parser_op_background(p , &wait);
    pipeline_set_wait(result , !wait);
    parser_garbage(p, &gar);

    if (pipeline_length(result) == 1 && scommand_length(pipeline_front(result)) == 0) { 
        pipeline_destroy(result);
        return NULL;
    }

    if (gar) {
        printf("Error: invalid command.\n");
        pipeline_destroy(result);
        return NULL;
    }

    return result;
}

