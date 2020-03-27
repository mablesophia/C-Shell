// shell.c
// name: wing wing

#include "headers.h"

void executable();

/* -----------------------------------------------------------------------------
 main
 usage: execute the shell
 -------------------------------------------------------------------------------*/
int main(void){
    setting_up_shell_environment();

    while(1) {
        if(signal(SIGCHLD,signal_handler)==SIG_ERR || signal(SIGINT,signal_handler)==SIG_ERR)
            perror("error: signal");

        int i,j;
        infile = outfile = NULL;
        is_bg = 0, num_pipe = 0;

        char** cmds = malloc((sizeof(char)*1024)*1024);
        char** cmd_tokens = malloc((sizeof(char)* 1024) * 1024);

        for(j = 0; j < 1024; j++) cmds[j] = '\0';
        printf("\n? ");

        char* cmdline = read_arg();

        for(i = 0; i < num_arg(cmdline, cmds); i++) {
            char* cmd_copy = strdup(cmds[i]);
            for(j = 0; j < 1024; j++) cmd_tokens[j] = '\0';
            if (redir_pipe_checking(strdup(cmds[i])) != -1)
                pipe_executing(cmds[i]);
            else{
                if (input_redir != 1 || output_redi != 1)
                    execute(parse_arg(strdup(cmds[i]), cmd_tokens), cmd_tokens, cmd_copy);
                else
                    execute(parse_redir(strdup(cmd_copy), cmd_tokens), cmd_tokens, cmd_copy);
            }
        }
        if(cmds) free(cmds);
        if(cmdline) free(cmdline);
    }
    return 0;
}

/* -----------------------------------------------------------------------------
 read_arg
 usage: read the user inputs, specify whether they are arguments, or comments(#),
        or escape characters(\).
 -------------------------------------------------------------------------------*/
char* read_arg(){
    int len=0,c;
    char* arg = malloc(sizeof(char)*1024);
    while(1) {
        c = getchar();
        if(c == '\n') {
            arg[len++] = '\0';
            break;
        }
        if(c == '#') arg[len++] = '\0';

        if(c == '\\') continue;

        else arg[len++] = c;
    }
    return arg;
}

/* -----------------------------------------------------------------------------
 num_arg
 usage: accept user input as argument and determine number of arguments in it
 -------------------------------------------------------------------------------*/
int num_arg(char* cmdline, char** cmds) {
    int num=0;
    char* token = strtok(cmdline, ";");
    while(token!=NULL) {
        cmds[num++] = token;
        token = strtok(NULL, ";");
    }
    return num;
}

/* -----------------------------------------------------------------------------
 execute
 usage: accept user input as argument and determine which function to execute
 -------------------------------------------------------------------------------*/
void execute(int tokens, char** cmd_tokens, char* cmd_copy){
    if(tokens > 0) {
        if(strcmp(cmd_tokens[tokens-1], "&") == 0) {
            is_bg = 1;
            cmd_tokens[tokens - 1] = NULL;
            execute_helper(cmd_tokens);
        }
        else if(strcmp(cmd_tokens[0], "fg") == 0 )  builtin(tokens, cmd_tokens);
        else if(strcmp(cmd_tokens[0], "cd") == 0)   change_dir(cmd_tokens, cwd, base_dir);
        else if(strcmp(cmd_tokens[0], "exit") == 0) _exit(0);
        else if(strcmp(cmd_tokens[0], "nsh") == 0)  execute_helper(cmd_tokens);
        else if(strcmp(cmd_tokens[0], "echo") == 0) echo_output(cmd_tokens, tokens, cmd_copy);
        else if(isalpha(cmd_tokens[0][0]))          execute_helper(cmd_tokens);

    }
    free(cmd_tokens);
}

/* -----------------------------------------------------------------------------
 execute_helper
 usage: helper function for execution
 -------------------------------------------------------------------------------*/
void execute_helper(char** cmd_tokens){
    pid_t pid; int file_in, file_out, status;
    pid = fork();

    if(pid==0) {
        setpgid(pid, pid);

        if(input_redir) {
            file_in = open_input_file();
            if(file_in == -1) _exit(-1);
        }
        if(output_redi) {
            file_out = open_output_file();
            if(file_out == -1) _exit(-1);
        }

        if(is_bg == 0) tcsetpgrp(shell, getpid());

        signal (SIGINT, SIG_DFL);  signal (SIGQUIT, SIG_DFL); signal (SIGTSTP, SIG_DFL);
        signal (SIGTTIN, SIG_DFL); signal (SIGTTOU, SIG_DFL); signal (SIGCHLD, SIG_DFL);

        if(execvp(cmd_tokens[0], cmd_tokens) < 0) _exit(-1);
        else _exit(0);
    }
    if(is_bg == 0) {
        tcsetpgrp(shell, pid);
        add_process(pid, cmd_tokens[0]);
        fgpid = pid;
        waitpid(pid, &status, WUNTRACED);

        if(!WIFSTOPPED(status)) reset_process(pid);
        tcsetpgrp(shell, my_pgid);
    }
    else {
        printf("\[%d] %d\n", num_jobs, pid);
        add_process(pid, cmd_tokens[0]);
    }
}

/* -----------------------------------------------------------------------------
 redir_pipe_checking
 usage: accepts arguments and check for existences of <, >, >> and |
 -------------------------------------------------------------------------------*/
int redir_pipe_checking(char* cmd){
    int i = 0; idxi = idxo = input_redir = output_redi = 0; pipe_count=0, redir2_count=0;

    while (cmd[i]) {
        if(cmd[i] == '|') pipe_count = 1;
        if(cmd[i] == '>') {
            output_redi = 1;
            if(redir2_count == 0) redir2_count = 1;
            if(idxo == 0 ) idxo = i;
        }
        if(cmd[i] == '>' && cmd[i+1] == '>') redir2_count = 2;
        if(cmd[i] == '<') {
            input_redir = 1;
            if(idxi == 0 ) idxi = i;
        }
        i++;
    }

    if(pipe_count) return 1;
    else return -1;
}

/* -----------------------------------------------------------------------------
 pipe_executing
 usage: accepts arguments and executing them with <, >, >> and/or |.
 -------------------------------------------------------------------------------*/
void pipe_executing(char* cmd){
    num_pipe = 0, is_bg=0; int pid, pgid = 0, file_in, file_out, status, i, j;
    parse_pipe(cmd);
    int* pipe_arr = (int* )malloc(sizeof(int)*(2*(num_pipe - 1)));

    for(i=0; i < 2*num_pipe - 3; i += 2) {
        if(pipe(pipe_arr+i) < 0 ) perror("error: pipe");
    }
    for(i=0; i < num_pipe; i++) {
        char** cmd_tokens = malloc((sizeof(char)*1024) * 1024);
        parse_redir(strdup(pipe_cmds[i]), cmd_tokens);

        pid = fork();
        if(i < num_pipe - 1) add_process(pid, cmd_tokens[0]);

        if(pid != 0 ) {
            if(i == 0 ) pgid = pid;
            setpgid(pid, pgid);
        }
        else if(pid == 0) {
            signal (SIGINT, SIG_DFL);  signal (SIGQUIT, SIG_DFL); signal (SIGTSTP, SIG_DFL);
            signal (SIGTTIN, SIG_DFL); signal (SIGTTOU, SIG_DFL); signal (SIGCHLD, SIG_DFL);

            if(output_redi) file_out = open_output_file();
            else if(i < num_pipe - 1) dup2(pipe_arr[2*i + 1], 1);

            if(input_redir) file_in = open_input_file();
            else if(i > 0 ) dup2(pipe_arr[2*i -2], 0);

            for(j = 0; j < 2*num_pipe - 2; j++) close(pipe_arr[j]);

            if(execvp(cmd_tokens[0], cmd_tokens) < 0 ) _exit(-1);
        }
    }

    for(i = 0; i < 2*num_pipe - 2; i++) close(pipe_arr[i]);

    if(is_bg == 0) {
        tcsetpgrp(shell, pgid);
        for(i = 0; i < num_pipe ; i++) {
            if(!WIFSTOPPED(status)) reset_process(waitpid(-pgid, &status, WUNTRACED));
        }
        tcsetpgrp(shell, my_pgid);
    }
}

/* -----------------------------------------------------------------------------
 builtin
 usage: execute basic builtin functions in the shell
 -------------------------------------------------------------------------------*/
void builtin(int tokens, char** cmd_tokens) {
    int count = atoi(cmd_tokens[1]), status;

    if(tokens != 2) perror("error: arg");
    if(table[count].active == 0) perror("error");

    pid_t pid;
    pid = fork();

    if(table[count].active == 1 && pid==0) {
        int pid = table[count].pid, pgid;
        pgid = getpgid(pid);
        tcsetpgrp(shell, pgid);
        fgpid = pgid;

        if(killpg(pgid, SIGCONT) < 0) perror("error");

        waitpid(pid, &status, WUNTRACED);
        if(!WIFSTOPPED(status)) {
            table[count].active = 0;
            fgpid = 0;
        }
        tcsetpgrp(shell, my_pid);
    }
}

/* -----------------------------------------------------------------------------
 echo_output
 usage: execute echo function in the shell
 -------------------------------------------------------------------------------*/
void echo_output(char** cmd_tokens, int tokens, char* cmd){
    if(tokens > 1 && cmd_tokens[1][0] == '-') {
        execute_helper(cmd_tokens);
        return;
    }
    int i, len = 0, in_quote = 0, flag = 0;
    char buf[1024] = "\0";
    for(i = 0; isspace(cmd[i]); i++);
    if(i == 0) i = 5;
    else i += 4;
    for(; cmd[i] != '\0' ; i++) {
        if(cmd[i] == '"' || cmd[i] == '\'') in_quote = 1 - in_quote;
        else if(in_quote == 0 && (isspace(cmd[i])) && flag == 0) {
            flag = 1;
            if(len > 0) buf[len++] = ' ';
        }
        else if(in_quote == 1 || !isspace(cmd[i])) buf[len++] = cmd[i];
        if(!isspace(cmd[i]) && flag == 1) flag = 0;
    }
    printf("%s\n", buf);
}

/* -----------------------------------------------------------------------------
 change_dir
 usage: change working directory in the shell
 -------------------------------------------------------------------------------*/
void change_dir(char** cmd_tokens, char* cwd, char* base_dir){
    if(chdir(cmd_tokens[1]) == 0) {
        getcwd(cwd, 1024);
        change_dir_helper(cwd);
    }
    else if(cmd_tokens[1] == NULL || strcmp(cmd_tokens[1], "~\0") == 0 || strcmp(cmd_tokens[1], "~/\0") == 0) {
        chdir(base_dir);
        strcpy(cwd, base_dir);
        change_dir_helper(cwd);
    }
    else perror("error: cd");
}

/* -----------------------------------------------------------------------------
 change_dir_helper
 usage: helper function for change_dir
 -------------------------------------------------------------------------------*/
void change_dir_helper(char* cwd){
    int i, j;
    for(i = 0; cwd[i]==base_dir[i] && cwd[i]!='\0' && base_dir[i] != '\0'; i++);
    if(base_dir[i] == '\0') {
        cwd[0] = '~';
        for(j = 1; cwd[i]!='\0'; j++) cwd[j] = cwd[i++];
        cwd[j] = '\0';
    }
}

/* -----------------------------------------------------------------------------
 executable //fail
 usage: run executable files in the shell
 -------------------------------------------------------------------------------*/
void executable(char** cmd_tokens){
    int status;
    pid_t filepid;
    filepid = fork();
    if(filepid != 0)
    {
        if(is_bg==0)
        {
            tcsetpgrp(shell, filepid);
            waitpid(filepid,&status, 0);
            tcsetpgrp(shell, my_pid);
        }
    }
    filepid = 0;
}

/* -----------------------------------------------------------------------------
signal_handler
 usage: handle signal(s) in the shell
 -------------------------------------------------------------------------------*/
void signal_handler(int signum) {
    int i=0, status, die_pid;

    if(signum == SIGCHLD) {

        while((die_pid = waitpid(-1, &status, WNOHANG)) > 0) {
            for(i = 0; i < num_jobs; i++){
                if(table[i].active==0) continue;
                else if(table[i].pid == die_pid) break;
            }
            if(i != num_jobs) {
                table[i].active = 0;
            }
        }
    }
    //else if(signum == SIGINT) signal(SIGINT, signal_handler);//
}

/* -----------------------------------------------------------------------------
 add_process
 usage: add process(es) in the shell
 -------------------------------------------------------------------------------*/
void add_process(int pid, char* name){
    table[num_jobs].pid = pid;
    table[num_jobs].name = strdup(name);
    table[num_jobs].active = 1;
    num_jobs++;
}

/* -----------------------------------------------------------------------------
 reset_process
 usage: reset process(es) in the shell
 -------------------------------------------------------------------------------*/
void reset_process(int pid){
    int i = 0;
    while (i<num_jobs) {
        if(table[i].pid == pid) {
            table[i].active = 0;
            break;
        }
        i++;
    }
}
/* -----------------------------------------------------------------------------
 open_input_file
 usage: reset process(es) in the shell
 -------------------------------------------------------------------------------*/
int open_input_file(){
    int f = open(infile, O_RDONLY, S_IRWXU);
    dup2(f, STDIN_FILENO);
    close(f);
    return f;
}
/* -----------------------------------------------------------------------------
 open_output_file
 usage: reset process(es) in the shell
 -------------------------------------------------------------------------------*/
int open_output_file(){
    int f = 0;

    if(redir2_count == 1)       f = open(outfile, O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU);
    else if(redir2_count == 2)  f = open(outfile, O_CREAT | O_WRONLY | O_APPEND, S_IRWXU);

    dup2(f, STDOUT_FILENO);
    close(f);
    return f;
}
/* -----------------------------------------------------------------------------
 setting_up_shell_environment
 usage: set up user directory, signal handler and calling process ID for shell
 -------------------------------------------------------------------------------*/
void setting_up_shell_environment(){
    shell = STDERR_FILENO; int i, j;

    input_cmd_tokens = malloc((sizeof(char)*1024) * 1024);
    output_cmd_tokens = malloc((sizeof(char)*1024)* 1024);

    if(isatty(shell)) {
        while(tcgetpgrp(shell) != (shell_pgid = getpgrp()))
        kill(shell_pgid, SIGTTIN);
    }

    signal (SIGTTIN, SIG_IGN); signal (SIGTTOU, SIG_IGN);

    my_pid = my_pgid = getpid(); setpgid(my_pid, my_pgid);
    tcsetpgrp(shell, my_pgid);

    getlogin_r(user, 1024-1); gethostname(hostname, 1024-1); getcwd(base_dir, 1024-1);
    strcpy(cwd, base_dir);

    for(i = 0; cwd[i]==base_dir[i] && cwd[i]!='\0' && base_dir[i] != '\0'; i++);
    if(base_dir[i] == '\0') {
        j = 1;
        cwd[0] = '~';
        while (cwd[i]!='\0') {
            cwd[j] = cwd[i++];
            j++;
        }
        cwd[j] = '\0';
    }

}
