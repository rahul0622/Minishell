/*
* Project Name : Mini Shell (msh)
 *
 * Description:
 * -------------
 * Mini Shell is a simplified implementation of the Linux BASH shell developed
 * using C and Linux system calls. The shell provides a command-line interface
 * to execute both built-in and external commands while demonstrating process
 * management, signal handling, and inter-process communication.
 *
 * Features:
 * ----------
 * • Customizable shell prompt using PS1.
 * • Execution of built-in commands:
 *      - cd
 *      - pwd
 *      - echo (supports $$, $?, and environment variables)
 *      - exit
 * • Execution of external commands using fork(), execvp(), and waitpid().
 * • Supports single as well as multiple commands connected through pipelines.
 * • Background process execution using '&'.
 * • Job control using:
 *      - jobs
 *      - fg
 *      - bg
 * • Signal handling:
 *      - SIGINT (Ctrl+C)
 *      - SIGTSTP (Ctrl+Z)
 *      - SIGCHLD for background process cleanup.
 * • Maintains background/stopped jobs using a doubly linked list.
 *
 * System Calls Used:
 * -------------------
 * fork(), execvp(), waitpid(), pipe(), dup2(), kill(), signal(),
 * getcwd(), chdir(), getpid(), getenv(), close()
 *
 * Author : Rahul S
*/

#include "header.h"
char prompt[20] = "minishell";
int main()
{
    char input_string[50];

    /* scanning the command line inputs */
    scan_input(prompt, input_string);

}