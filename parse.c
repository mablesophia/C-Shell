// parse.c
// name: wing wing

#include "headers.h"

/* -----------------------------------------------------------------------------
 parse_arg
 usage: parse the user argument(s) in the shell
 -------------------------------------------------------------------------------*/
int parse_arg(char* cmd, char** cmd_tokens){
    int tok = 0;
    char* token = strtok(cmd, " \t\n");
    while(token!=NULL) {
        cmd_tokens[tok++] = token;
        token = strtok(NULL, " \t\n");
    }
    return tok;
}

/* -----------------------------------------------------------------------------
 parse_redir
 usage: further parse user argument(s) that contains redirection symbol(s)
 -------------------------------------------------------------------------------*/
int parse_redir(char* cmd, char** cmd_tokens){
    char* copy = strdup(cmd);
    idxi = idxo = input_redir = output_redi = 0;
    infile = outfile = NULL;
    int i=0, tok = 0; redir2_count=0;

    for( i = 0 ; cmd[i] ; i++) {
        if(cmd[i] == '<') {
            input_redir = 1;
            if(idxi == 0 ) idxi = i;
        }
        if(cmd[i] == '>') {
            output_redi = 1;
            if(redir2_count == 0) redir2_count = 1;
            if(idxo == 0 ) idxo = i;
        }
        if(cmd[i] == '>' && cmd[i+1] == '>') redir2_count = 2; printf(" >> exists\n");
    }
    if(input_redir == 1 && output_redi == 1) {
        char* token;
        token = strtok(copy, " <>\t\n");
        while(token!=NULL) {
            cmd_tokens[tok++] = strdup(token);
            token = strtok(NULL, "<> \t\n");
        }
        if(idxi < idxo ) {
            infile = strdup(cmd_tokens[tok - 2]);
            outfile = strdup(cmd_tokens[tok - 1]);
        }
        else {
            infile = strdup(cmd_tokens[tok - 1]);
            outfile = strdup(cmd_tokens[tok - 2]);
        }
        cmd_tokens[tok - 2] = cmd_tokens[tok - 1] = NULL;

        return tok - 2;
    }

    if(input_redir == 1) {
        char* token;
        char* copy = strdup(cmd);

        char** input_redi_cmd = malloc((sizeof(char)*MAX_BUF_LEN)*MAX_BUF_LEN);
        token = strtok(copy, "<");
        while(token!=NULL) {
            input_redi_cmd[tok++] = token;
            token = strtok(NULL, "<");
        }
        copy = strdup(input_redi_cmd[tok - 1]);

        token = strtok(copy, "> |\t\n");
        infile = strdup(token);

        tok = 0;
        token = strtok(input_redi_cmd[0], CMD_DELIMS);
        while(token!=NULL) {
            cmd_tokens[tok++] = strdup(token);
            token = strtok(NULL, CMD_DELIMS);
        }

        cmd_tokens[tok] = NULL;

        free(input_redi_cmd);
    }

    if(output_redi == 1) {
        char* copy = strdup(cmd);
        char* token = NULL;
        char** output_redi_cmd = malloc((sizeof(char)*MAX_BUF_LEN)*MAX_BUF_LEN);
        if(redir2_count == 1) token = strtok(copy, ">");
        else if(redir2_count == 2)
        token = strtok(copy, ">>");
        while(token!=NULL) {
            output_redi_cmd[tok++] = token;
            if(redir2_count == 1) token = strtok(NULL, ">");
            else if(redir2_count == 2) token = strtok(NULL, ">>");
        }

        copy = strdup(output_redi_cmd[tok - 1]);
        token = strtok(copy, "< |\t\n");
        outfile = strdup(token);

        tok = 0;
        token = strtok(output_redi_cmd[0], CMD_DELIMS);
        while(token!=NULL) {
            cmd_tokens[tok++] = token;
            token = strtok(NULL, CMD_DELIMS);
        }

        free(output_redi_cmd);
    }
    if(input_redir == 0 && output_redi == 0 ) return parse_arg(strdup(cmd), cmd_tokens);
    else return tok;
}

/* -----------------------------------------------------------------------------
 parse_pipe
 usage: further parse user argument(s) that contains pipe symbol(s)
 -------------------------------------------------------------------------------*/
void parse_pipe(char* cmd){
    int token_count=0;
    char* copy = strdup(cmd);
    char* token = strtok(copy, "|");
    while(token!= NULL) {
        pipe_cmds[token_count++] = token;
        token = strtok(NULL, "|");
    }
    num_pipe = token_count;
}
