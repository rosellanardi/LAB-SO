#include <assert.h>
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include "strextra.h"
#include "command.h"

struct scommand_s {
    GQueue *command;
    char *out;
    char *in;
};

scommand scommand_new(void){
	scommand new = malloc(sizeof(struct scommand_s));
    
    new-> command = g_queue_new();
    new-> out = NULL;
    new-> in = NULL;
    
    assert(new != NULL && scommand_is_empty (new) && scommand_get_redir_in (new) == NULL &&
    scommand_get_redir_out (new) == NULL);	
    return new;
}

void free_item(gpointer data);

void free_item(gpointer data) {
    free(data);
}

scommand scommand_destroy(scommand self){
	assert(self != NULL);
    g_queue_free_full(self->command, free_item);
    free(self->out);
    free(self->in);
    free(self);
   	self = NULL;
    assert(self == NULL);
    return self;
}

void scommand_push_back(scommand self, char * argument){
	assert(self != NULL && argument != NULL);
	g_queue_push_tail(self->command, argument);
	assert(!scommand_is_empty(self));
}

void scommand_pop_front(scommand self){
	assert(self != NULL && !scommand_is_empty(self));
    gpointer item = g_queue_pop_head(self->command);
    free(item);
}

void scommand_set_redir_in(scommand self, char * filename){
	assert(self != NULL);
	free(self->in);
	self->in = filename; 
}

void scommand_set_redir_out(scommand self, char * filename){
	assert(self != NULL);
	free(self->out);
    self->out = filename;
}

bool scommand_is_empty(const scommand self){
	assert(self != NULL);
	return (g_queue_get_length(self->command) == 0);
}

unsigned int scommand_length(const scommand self){
	assert(self != NULL);
    unsigned int res = g_queue_get_length(self->command);
    assert((res == 0) == scommand_is_empty(self));
    return res;
}


char * scommand_front(const scommand self){
	assert(self != NULL && !scommand_is_empty(self));
	char *cmd = NULL;
	cmd = g_queue_peek_head(self->command);
	assert(cmd != NULL);
	return cmd;
}

char * scommand_get_redir_in(const scommand self){
	assert(self != NULL);
    return self->in;
}

char * scommand_get_redir_out(const scommand self){
	assert(self != NULL);
    return self->out;

}

char * scommand_to_string(const scommand self){
	assert(self != NULL);
	
	char *result = NULL;
	char *temp = NULL;
	
	if (scommand_length(self) > 0) {
		result = strdup(scommand_front(self));
	} else {
		result = strdup("");
	}

	for (unsigned int n = 1; n < scommand_length(self); n++) {
		temp = strmerge(result, " ");
		free(result);
		result = temp;
		temp = strmerge(result, g_queue_peek_nth(self->command, n));
		free(result);
		result = temp;
	}
	
	if (self->out != NULL) {
		temp = strmerge(result, " > ");
		free(result);
		result = temp;
		temp = strmerge(result, scommand_get_redir_out(self));
		free(result);
		result = temp;
	}
	if (self->in != NULL) {
		temp = strmerge(result, " < ");
		free(result);
		result = temp;
		temp = strmerge(result, scommand_get_redir_in(self));
		free(result);
		result = temp;
	} 
	assert(scommand_is_empty(self) || scommand_get_redir_in(self)== NULL || 
		scommand_get_redir_out(self) == NULL || strlen(result) > 0);
		
	return result;
}

struct pipeline_s {
    GQueue *scommands;
    bool wait;
};

pipeline pipeline_new(void){
	pipeline new_pipeline = malloc(sizeof(struct pipeline_s));

	if (new_pipeline == NULL) {
		fprintf(stderr, "Error");
		exit(EXIT_FAILURE);
	}

	new_pipeline->scommands = g_queue_new();
	new_pipeline->wait = true;

	assert(pipeline_is_empty(new_pipeline) && pipeline_get_wait(new_pipeline));
	return new_pipeline;
}


pipeline pipeline_destroy(pipeline self){
	assert(self != NULL);
	while (!g_queue_is_empty(self->scommands)) {
		pipeline_pop_front(self);
	}
	g_queue_free(self->scommands);
	free(self);
	self = NULL;
	assert(self == NULL);
	return self;
}


void pipeline_push_back(pipeline self, scommand sc){
	assert (self != NULL && sc != NULL);
	g_queue_push_tail(self->scommands, sc);
	assert(!pipeline_is_empty(self));
}


void pipeline_pop_front(pipeline self){
	assert (self != NULL && !pipeline_is_empty(self));
 	scommand sc = g_queue_pop_head(self->scommands);
 	scommand_destroy(sc);
 	
}

void pipeline_set_wait(pipeline self, const bool w){
	assert(self != NULL);
	self->wait=w;

}

bool pipeline_is_empty(const pipeline self){
    assert(self != NULL);
    return (g_queue_is_empty(self->scommands));
}

unsigned int pipeline_length(const pipeline self){
    assert(self != NULL);
    unsigned int length = (g_queue_get_length(self->scommands));
    assert((length == 0) == pipeline_is_empty(self));
    return length;
}


scommand pipeline_front(const pipeline self){
	assert(self != NULL && !pipeline_is_empty(self));
    scommand command = g_queue_peek_head(self->scommands);
    assert(command != NULL);
    return command;
}


bool pipeline_get_wait(const pipeline self){
	assert(self != NULL);
	return (self->wait);
}


char * pipeline_to_string(const pipeline self){
    assert(self != NULL);

    unsigned int length = 0;
    char * result = NULL;
    char * temp = NULL;
    char * command_string = NULL;
    scommand command = NULL;

    length = g_queue_get_length(self->scommands);

    if (length == 0) {
    	result = strdup("");
    } else {
    	command = g_queue_peek_head(self->scommands);
    	command_string = scommand_to_string(command);
    	result = strmerge(command_string, " | ");
    	free(command_string);
    }

    for (unsigned int i=1; i < length; i++) {
        command = g_queue_peek_nth(self->scommands, i);
        command_string = scommand_to_string(command);
        temp = strmerge(result,command_string);
        free(result);
        result = temp;

        if (i != length -1) {
            temp = strmerge(result, " | ");
            free(result);
    		result = temp;
        } 
        free(command_string);
        
    }
    
    if (!pipeline_get_wait(self)) {
    	temp = strmerge(result, " & ");
    	free(result);
    	result = temp;
    }
    assert(pipeline_is_empty(self) || pipeline_get_wait(self) || strlen(result)>0);

    return result;
}
