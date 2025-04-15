#include <assert.h>
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>

#include "builtin.h"
#include "execute.h"
#include "tests/syscall_mock.h"

void execute_pipeline(pipeline apipe) {
	assert(apipe != NULL);

	if (pipeline_is_empty(apipe)) {
		return;
	} else if (builtin_alone(apipe)) {
		builtin_run(pipeline_front(apipe));
		return;
	}

	unsigned int pipe_length = pipeline_length(apipe);
	int id_children[pipe_length];
	scommand sc = NULL;

	if (pipe_length == 1) {
		int new_proc = fork();
		if (new_proc == 0) {
			sc = pipeline_front(apipe);

			execute_scommand(sc);
			fprintf(stderr, "Error al ejecutar un comando.\n");
			exit(EXIT_FAILURE);
		}
		if (pipeline_get_wait(apipe)) {
			waitpid(new_proc, NULL, 0);
		} else {
			signal(SIGCHLD, SIG_IGN);
		}
		
	} else {
        int pipefd[pipe_length - 1][2];

		for (unsigned int i = 0; i < pipe_length; i++) {

			if (i != pipe_length - 1) {
				int res = pipe(pipefd[i]);
        
	        	if (res == -1) {
	            	exit(EXIT_FAILURE);
	        	}
			}

			int new_proc = fork();

			if (new_proc == 0) {
				sc = pipeline_front(apipe);

				if (strcmp(scommand_to_string(sc), "") == 0) {
					printf("Error, es necesario otro comando.\n");
					pipeline_destroy(apipe);
					exit(EXIT_FAILURE);
				}


				if (i != pipe_length - 1) {
	                close(pipefd[i][0]);
	                dup2(pipefd[i][1], STDOUT_FILENO);
	                close(pipefd[i][1]);
	                 		    
				}
				if (i != 0) {
				    close(pipefd[i-1][1]);
	                dup2(pipefd[i-1][0], STDIN_FILENO);
	                close(pipefd[i-1][0]);
	            }				
				
				execute_scommand(sc);
				fprintf(stderr, "Error al ejecutar un comando.\n");
				exit(EXIT_FAILURE);

			} else if ( new_proc < 0) {
				fprintf(stderr, "Error fork");
				return;
			} else {
				if (i != 0) {
					close(pipefd[i-1][0]);
					close(pipefd[i-1][1]);
				}
			}

			id_children[i] = new_proc;
			pipeline_pop_front(apipe);
		}

		if (pipeline_get_wait(apipe)) {
			for (unsigned int i = 0; i < pipe_length; i ++) {
				waitpid(id_children[i], NULL, 0);
			}
		} else {
			signal(SIGCHLD, SIG_IGN);
		}
	}
}

void execute_scommand(scommand sc) {
	assert(sc != NULL);

	char * filename_in = NULL;
	char * filename_out = NULL;
	unsigned int sc_length = scommand_length(sc);

	char **argv = calloc(sc_length + 1, sizeof(char *));

	for (unsigned int i = 0; i < sc_length; i++) {
		argv[i] = strdup(scommand_front(sc));
		scommand_pop_front(sc);
	}

	filename_in = scommand_get_redir_in(sc);
	if (filename_in) {
		int file_in = open(filename_in, O_RDONLY, S_IWUSR);
		close(STDIN_FILENO);
		dup2(file_in, STDIN_FILENO);
		close(file_in);
	}

	filename_out = scommand_get_redir_out(sc);
	if (filename_out) {
		int file_out = open(filename_out, O_CREAT | O_WRONLY, S_IWUSR | S_IRUSR );
		close(STDOUT_FILENO);
		dup2(file_out, STDOUT_FILENO);
		close(file_out);
	}

	argv[sc_length] = NULL;
	execvp(argv[0], argv);

	// Si falla el comando, el hijo elimina toda la memoria.
	for (unsigned int i = 0; i < sc_length; i++) {
		free(argv[i]);
	}
	free(argv);
	scommand_destroy(sc);
}
