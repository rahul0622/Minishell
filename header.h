#ifndef MAIN_H
#define MAIN_H
 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdio_ext.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

typedef struct job{
    pid_t pid;
    int status;
    int id;
    char cmd[50];

    struct job *prev;
    struct job *next;
}JOB;

#define BUILTIN		1
#define EXTERNAL	2
#define NO_COMMAND  3
#define RUNNING     4
#define STOPPED     5

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"


void scan_input(char *prompt, char *input_string);
char *get_command(char *input_string);

void job(char *input_string);
void sigchld_handler(int sig);
void delete_job(pid_t pid);
void print_job();
void print_single_job(pid_t pid);
void insert_job(pid_t pid, char *cmd, int status);

int check_command_type(char *command);
void echo(char *input_string, int status);
void execute_internal_commands(char *input_string);
void signal_handler(int signum);
void execute_external_commands(char *external_commands);

#endif