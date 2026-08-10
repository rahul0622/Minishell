#include "header.h"

extern char prompt[20];
/* external cmd child creation varible */
pid_t pid;

JOB *head = NULL;
JOB *tail = NULL;
int id = 0;

void scan_input(char *prompt, char *input_string)
{
    system("clear");

    /* register the signals */
    signal(SIGINT, signal_handler);
    signal(SIGTSTP, signal_handler);
    signal(SIGCHLD, sigchld_handler);
    
    while(1)
    {
        char temp[50];

        printf(ANSI_COLOR_GREEN "%s"ANSI_COLOR_RESET":",prompt);

        /* read the input command */
        fgets(input_string, 50, stdin);
        input_string[strcspn(input_string, "\n")] = '\0';
        strcpy(temp, input_string);

        /* check for PS1 command */
        if(strncmp(temp, "PS1=", 4) == 0)
        {
            strcpy(prompt, temp + 4);
        }
        /* check for clear command */
        else if(strcmp(temp, "clear") == 0)
        {
            system("clear");
        }
        /* check fro jobs command */
        else if(strstr(temp, "jobs") != NULL)
        {
            print_job();
        }
        /* check for foreground and background process */
        else if(strstr(temp, "fg") != NULL || strstr(temp, "bg") != NULL)
        {
            job(input_string);
        }
        /* check for process command to run as background process */
        else if(strchr(temp, '&') != NULL)
        {
            char cmd_copy[50];
            strcpy(cmd_copy, temp);

            char *spot = strchr(temp, '&');
            *spot = '\0';

            char *arg[20];
            int i = 0;
            arg[i] = strtok(temp, " ");

            while(arg[i] != NULL)
            {
                i++;
                arg[i] = strtok(NULL, " ");
            }
            
            pid = fork();
            if(pid == 0)
            {
                execvp(arg[0], arg);
                perror(ANSI_COLOR_RED"execvp"ANSI_COLOR_RESET);
                exit(1);
            }
            else
            {
                insert_job(pid, cmd_copy, RUNNING);
            }
        }
        else
        {
            /* get first command */
            char *cmd = get_command(temp);
            /* identify the type of command */
            int ret = check_command_type(cmd);

            switch(ret)
            {
                case BUILTIN:
                execute_internal_commands(input_string);
                break;

                case EXTERNAL:
                execute_external_commands(input_string);
                break;

                case NO_COMMAND:
                printf("INVAILD COMMAND\n");
                break;
            }
        }
   }
}

char *get_command(char *input_string)
{
    /* return word by word from input string */
    return strtok(input_string, " ");
}

int check_command_type(char *command)
{
    char *builtins[] = {"echo", "printf", "read", "cd", "pwd", "pushd", "popd", "dirs", "let", "eval",
						"set", "unset", "export", "declare", "typeset", "readonly", "getopts", "source",
						"exit", "exec", "shopt", "caller", "true", "type", "hash", "bind", "help", NULL};
    
    char *external[] ={"bash", "bunzip2", "busybox", "bzcat", "bzcmp", "bzdiff", "bzegrep", "bzexe", "bzfgrep", "bzgrep",
                        "bzip2", "bzip2recover", "bzless", "bzmore", "cat", "chacl", "chgrp", "chmod", "chown", "chvt", "cp",
                        "cpio", "dash", "date", "dbus-cleanup-sockets", "dbus-daemon", "dbus-uuidgen", "dd", "df", "dir", "dmesg",
                        "dnsdomainname", "domainname", "dumpkeys", "echo", "ed", "egrep", "false", "fgconsole", "fgrep", "findmnt",
                        "fuser", "fusermount", "getfacl", "grep", "gunzip", "gzexe", "gzip", "hostname", "ip", "kbd_mode", "kill",
                        "kmod", "less", "lessecho", "lessfile", "lesskey", "lesspipe", "ln", "loadkeys", "login", "loginctl", "lowntfs-3g",
                        "ls", "lsblk", "lsmod", "mkdir", "mknod", "mktemp", "more", "mount", "mountpoint", "mt", "mt-gnu", "mv", "nano",
                        "nc", "nc.openbsd", "netcat", "netstat", "nisdomainname", "ntfs-3g", "ntfs-3g.probe", "ntfs-3g.secaudit",
                        "ntfs-3g.usermap", "ntfscat", "ntfsck", "ntfscluster", "ntfscmp", "ntfsdump_logfile", "ntfsfix", "ntfsinfo",
                        "ntfsls", "ntfsmftalloc", "ntfsmove", "ntfstruncate", "ntfswipe", "open", "openvt", "pidof", "ping", "ping6",
                        "plymouth", "plymouth-upstart-bridge", "ps", "pwd", "rbash", "readlink", "red", "rm", "rmdir", "rnano", "running-in-container",
                        "run-parts", "sed", "setfacl", "setfont", "setupcon", "sh", "sh.distrib", "sleep", "ss", "static-sh", "stty", "su",
                        "sync", "tailf", "tar", "tempfile", "touch", "true", "udevadm", "ulockmgr_server", "umount", "uname", "uncompress",
                        "unicode_start", "vdir", "vmmouse_detect", "wc", "which", "whiptail", "ypdomainname", "zcat", "zcmp", "zdiff", "zegrep",
                        "zfgrep", "zforce", "zgrep", "zless", "zmore", "znew", NULL};
    int i = 0;

    /* return the builtin commands */
    while(builtins[i] != NULL)
    {
        if(strcmp(builtins[i], command) == 0)
            return BUILTIN;
        i++;
    }

    i = 0;

    /* return the external commands */
    while(external[i] != NULL)
    {
        if(strcmp(external[i], command) == 0)
            return EXTERNAL;

        i++;
    }
    return NO_COMMAND;
}

void execute_internal_commands(char *input_string)
{
    char temp[50];
    strcpy(temp, input_string);

    char *cmd = strtok(temp," ");

    /* check for cd command */
    if(strcmp(cmd, "cd") == 0)
    {
        char *path = strtok(NULL, " ");

        if(path == NULL)
        {
            path = getenv("HOME");
        }
        if(chdir(path) == -1)
        {
            perror(ANSI_COLOR_RED"cd"ANSI_COLOR_RESET);
        }
    }
    /* check for pwd command */
    else if(strcmp(cmd, "pwd") == 0)
    {
        char cwd[1024];

        if (getcwd(cwd, sizeof(cwd)) != NULL)
            printf("%s\n", cwd);
        else
            perror(ANSI_COLOR_RED"pwd"ANSI_COLOR_RESET);
    }
    /* exit from minishell */
    else if(strcmp(cmd, "exit") == 0)
    {
        exit(EXIT_SUCCESS);
    }
    /* check for echo commands */
    else if(strcmp(cmd, "echo") == 0)
    {
        int status = 0;
        echo(input_string, status);
    }
    else
    {
        printf(ANSI_COLOR_RED"Buitlin-command not executable\n"ANSI_COLOR_RESET);
    }
}

void echo(char *input_string, int status)
{
    char temp[50];
    strcpy(temp, input_string);

    strtok(temp, " ");          // Skip "echo"

    char *arg = strtok(NULL, "");

    /* only echo command */
    if(arg == NULL)
    {
        printf("\n");
    }
    /* echo for a particular process search */
    else if(strcmp(arg, "$$") == 0)
    {
        printf("%d\n", getpid());
    }
    /* echo for status of last command executed */
    else if(strcmp(arg, "$?") == 0)
    {
        printf("%d\n", status);
    }
    /* echo to display the path */
    else if(arg[0] == '$')
    {
        char *value = getenv(arg + 1);

        if(value)
            printf("%s\n", value);
    }
    /* only echo command execution */
    else
    {
        printf("%s\n", arg);
    }
}

void execute_external_commands(char *external_commands)
{
    /* if pipe command used */
    int pipe_count = 0;

    for(int i = 0; external_commands[i] != '\0'; i++)
    {
        if(external_commands[i] == '|')
            pipe_count++;
    }
    /* creation of pipes */
    int cmd_count = pipe_count + 1;
    int pipe_fd[pipe_count][2];

    for(int i = 0; i < pipe_count; i++)
    {
        if(pipe(pipe_fd[i]) == -1)
        {
            perror(ANSI_COLOR_RED"pipe\n"ANSI_COLOR_RESET);
            return;
        }
    }

    char cmd_copy[100];
    strcpy(cmd_copy, external_commands);

    /* cmd splitting */
    char *cmd[cmd_count];
    cmd[0] = strtok(external_commands, "|");

    for(int i = 1; i < cmd_count; i++)
    {
        cmd[i] = strtok(NULL, "|");

        while(*cmd[i] == ' ')
            cmd[i]++;
    }

    int status;
    /* create child */
    for(int i = 0; i < cmd_count; i++)
    {
        /* create child */
        pid = fork();
        if(pid == -1)
        {
            perror(ANSI_COLOR_RED"fork"ANSI_COLOR_RESET);
            return;
        }

        if(pid == 0)
        {
            /* default signal handling */
            signal(SIGINT, SIG_DFL);
            signal(SIGTSTP, SIG_DFL);

            /* multiple commands*/
            if(i > 0)
            dup2(pipe_fd[i - 1][0], STDIN_FILENO);

            /* single command */
            if(i < pipe_count)
            dup2(pipe_fd[i][1], STDOUT_FILENO);

            /* close unwanted pipe ends */
            for(int j = 0; j < pipe_count; j++)
            {
                close(pipe_fd[j][0]);
                close(pipe_fd[j][1]);
            }
            /* execute the cmd */
            char *argv[20];
            int k = 0;

            argv[k] = strtok(cmd[i], " ");

            while(argv[k] != NULL)
            {   
                k++;
                argv[k] = strtok(NULL, " ");
            }
            execvp(argv[0], argv);
            perror(ANSI_COLOR_RED"execvp"ANSI_COLOR_RESET);
            _exit(1);
        }
    }
    /* close unwanted pipe ends */
    for (int i = 0; i < pipe_count; i++)
    {
        close(pipe_fd[i][0]);
        close(pipe_fd[i][1]);
    }
    /* wait for each created child */
    for (int i = 0; i < cmd_count; i++)
    {
        waitpid(pid, &status, WUNTRACED);
        if(WIFSTOPPED(status))
        {
            insert_job(pid, cmd_copy, STOPPED);
        }
    }
    /* as globally declared */
    pid = 0;
}

void signal_handler(int signum)
{
    /* user handling of SIGINT signal */
    if(signum == SIGINT)
    {
        if(pid == 0)
        {
            printf("\n");
            printf(ANSI_COLOR_GREEN"%s"ANSI_COLOR_RESET":",prompt);
        }
        else{
            printf("\n");
        }
    }
    /* user handling of SIGTSTP signal */
    else if(signum == SIGTSTP)
    {
        if(pid == 0)
        {
            printf("\n");
            printf(ANSI_COLOR_GREEN"%s"ANSI_COLOR_RESET":",prompt);
        }
        else{
            printf("\n");
        }
    }
    /* flush the output */
    fflush(stdout);
}

void job(char *input_string)
{
    /* to execute the foreground processes */
    if(strcmp(input_string, "fg"))
    {
        if(head == NULL)
        {
            printf(ANSI_COLOR_RED"No jobs\n"ANSI_COLOR_RESET);
            return;
        }

        kill(head->pid, SIGCONT);

        int status;

        waitpid(head->pid, &status, WUNTRACED);

        if(WIFSTOPPED(status))
        {
            head->status = STOPPED;
        }
        else
        {
            delete_job(head->pid);
        }
    }
    /* to execute the background process */
    else if(strcmp(input_string, "bg"))
    {
        if(head == NULL)
        {
            printf(ANSI_COLOR_RED"No jobs\n"ANSI_COLOR_RESET);
            return;
        }

        kill(head->pid, SIGCONT);
        head->status = RUNNING;

        print_single_job(head->pid);
    }
}

void sigchld_handler(int sig)
{
    /* SIGCHILD handling for job deletion */
    int status;
    pid_t child_pid;

    while((child_pid = waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0)
    {
        if(WIFEXITED(status) || WIFSIGNALED(status))
        {
            delete_job(child_pid);
        }
    }
}

void insert_job(pid_t pid, char *cmd, int status)
{
    /* to add a uncompleted process as jobs */
    JOB *new = malloc(sizeof(JOB));
    if(!new)
        return;
    
    new->pid = pid;
    new->id = ++id;
    strcpy(new->cmd, cmd);
    new->status = status;
    new->prev = NULL;
    new->next = NULL;

    if(head == NULL)
    {
        head = new;
        tail = new;
        new->next = NULL;
        return;
    }
    head->prev = new;
    new->next = head;
    head = new;
    return;
}

void print_single_job(pid_t pid)
{
    /* to display the complted jobs from fg and bg commands */
    JOB *temp = head;

    while(temp)
    {
        if(temp->pid == pid)
        {
            char *run = "Runnning";
            char *stop = "Stopped";
            
            printf(ANSI_COLOR_YELLOW"[%d]  %s            %s\n"ANSI_COLOR_RESET, temp->id, temp->status == RUNNING ? run : stop, temp->cmd);
            return;
        }
        temp = temp->next;
    }
}

void print_job()
{
    /* to display for jobs commands */
    if(head == NULL)
    {
        return;
    }
    else
    {
        char *run = "Runnning";
        char *stop = "Stopped";
        JOB *temp = tail;
        while(temp)
        {
            printf("[%d]  %s            %s\n", temp->id, temp->status == RUNNING ? run : stop, temp->cmd);
            temp = temp->prev;
        }
    }
}

void delete_job(pid_t pid)
{
    /* delete the completed jobs from fg and bg commands */
    if(head == NULL)
    {
        return;
    }
    JOB *temp = head;

    while(temp)
    {
        if(temp->pid == pid)
        {
            if(temp->prev)
                temp->prev->next = temp->next;
            else
                head = temp->next;
            
            if(temp->next)
                temp->next->prev = temp->prev;
            else
                tail = temp->prev;

            free(temp);
            return;
        }
        temp = temp->next;
    }
    id--;
}