// #ifndef _POSIX_C_SOURCE
//     #define _POSIX_C_SOURCE 200809L
// #endif

#include <fcntl.h>
#include <readline/history.h>
#include <readline/readline.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>  //posix os api //sleep

#define MAXCOM 1000     // max number of letters to be supported
#define MAXLIST 100     // max number of commands to be supported
#define Last_16_CMD 16  // last 16 command

// Clearing the shell using escape sequences
#define clear() printf("\033[H\033[J")

// Greeting shell during startup
void init_shell() {  // 1.0 requirement hello-message
    clear();
    printf("======================================================\n");
    printf("\t****WELLCOME TO MY SHELL****\n");
    printf("\tWhy OS HW1 this year is so complicate?\n");
    printf("\t來試試看用中文會怎樣\n");
    printf("======================================================\n");
    // char* username = getenv("USER");
    // printf("\n\n\nUSER is: @%s", username);
    // printf("\n");
    // sleep(10);
    // clear();
}

// Function to take input
int takeInput(char* str) {
    char* buf;
    char* is_replay = "replay";

    buf = readline(">>> $ ");  // 1.0 requirement You must print prompt symbol!!!
    // printf("pid>%ld\n", (long)getpid());
    if (strlen(buf) != 0) {
        // printf("buffff %s\n",buf);
        if (strstr(buf, is_replay) == NULL) {  // 2.2 requirement don't save replay
            add_history(buf);
        }
        strcpy(str, buf);  // copy to str(point to inputString)
        // printf("x = %x, s = %s, p = %p", str, str, str);  // print what user input again  //記得註解掉

        // 1.2 requirement
        // strcmp for string pointer compare if = return 0
        // ctrl+v and then tab
        if (strcmp(str, " ") == 0 || strcmp(str, "\t") == 0) {
            // printf("1.2 requirement111");
            return 1;
        } else {
            // printf("1.2 requirement000");
            return 0;
        }
    } else {
        // printf("1.2 requirement1");
        return 1;
    }
}

// check redirection and background
int redirection_background_check(char** parsed, int* redirection_mode, int* run_background) {
    // check redirection_mode: 0 no, 1 = <, 2 = >, 3 = <+>
    int i;

    for (i = 0; i < MAXLIST; i++) {
        if (parsed[i] == NULL) {
            i--;
            break;
        }
    }
    // printf("i %d\n", i);
    if (i > 1) {
        if (!strcmp(parsed[1], "<")) {  // if have >
            *redirection_mode = 1;
        }

        // printf("how much parsed: %d\n", i);
        if (!strcmp(parsed[i], "&")) {  // if have &
            *run_background = 1;
            i--;
        }
        i--;
        if (!strcmp(parsed[i], ">")) {  // if have >

            if (*redirection_mode == 1) {
                *redirection_mode = 3;  //<+>
            } else {
                *redirection_mode = 2;  //>
            }
        }
        // printf("redirection mode: %d\n", redirection_mode);
        // printf("where > is: %d\n", i);
        if (*redirection_mode == 1 || *redirection_mode == 3) {
            parsed[1] = NULL;  // cleanup <
        }
        if (*redirection_mode == 2 || *redirection_mode == 3) {
            parsed[i] = NULL;  // cleanup >
        }
        // printf("i %d\n", i);
        if (*run_background == 1) {
            i += 2;
            parsed[i] = NULL;  // cleanup &
        }
    } else if (i != 0) {  //只有一個指令檢查有沒有&，ex: ls & , ls -l
        if (!strcmp(parsed[1], "&")) {
            *run_background = 1;
            parsed[1] = NULL;  // cleanup &
        }
    }
    // printf("run back%d\n", *run_background);
    return i;
}
// Function where the system command is executed
void execArgs(char** parsed) {
    char* built_in_str[1];  // be used to store strings return from builtin
    // check redirection_mode: 0 no, 1 = <, 2 = >, 3 = <+>
    int i;  // get how many cmds
    int redirection_mode = 0, run_background = 0;
    i = redirection_background_check(parsed, &redirection_mode, &run_background);
    // printf("i%d\n", i);
    // printf("run back%d\n", run_background);

    // printf("???\n");
    // printf("%s %s %s %s %s\n", parsed[0], parsed[1], parsed[2], parsed[3], parsed[4]);
    int flag = 1;
    flag = ownCmdHandler(parsed, built_in_str);
    // printf("flag = %d\n", flag);
    // printf("%s", built_in_str[0]);

    // int j;
    // for (j = 0; j < MAXCOM; j++) {
    //     if (built_in_str[j] == NULL)
    //         break;
    //     else
    //         printf("%s", built_in_str[j]);
    // }
    if (flag == 2) {  // help, echo...
        if (!run_background) {
            i++;
        } else {
            i--;
        }
        int fd1, fd2;
        if (redirection_mode == 2) {
            /* Connect standard output to given file */
            fflush(stdout);
            fd1 = open(parsed[i], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd1 < 0)
                printf("Failed to open %s for writing\n", parsed[i]);
            fd2 = dup(STDOUT_FILENO);
            if (fd2 < 0)
                printf("Failed to duplicate standard output\n");
            if (dup2(fd1, STDOUT_FILENO) < 0)
                printf("Failed to duplicate %s to standard output\n", parsed[i]);
            close(fd1);
        }
        /* Write to standard output */
        printf("%s", built_in_str[0]);
        if (redirection_mode == 2) {
            /* Reconnect original standard output */
            fflush(stdout);
            if (dup2(fd2, STDOUT_FILENO) < 0)
                printf("Failed to reinstate standard output\n");
            close(fd2);
        }
    }

    if (flag == 0) {  // unexecuted
        // Forking a child //-1 ： 發生錯誤 // 0 ： 代表為子程序 //大於 0 ： 代表為父程序, 其回傳值為子程序的 ProcessID
        pid_t pid = fork();
        // printf("pid>%ld, ppid>%ld\n", (long)getpid(), (long)getppid());
        if (pid == -1) {  // fork error
            printf("\nFailed forking child..");
            return;

        } else if (pid == 0) {  // 0 ： 代表為子程序

            if (redirection_mode == 1 || redirection_mode == 3) {
                int fd0 = open(parsed[2], O_RDONLY);  // parsed[2],the file after <
                dup2(fd0, STDIN_FILENO);              // parsed[2],closed
                close(fd0);
            }

            if (redirection_mode == 2 || redirection_mode == 3) {
                // printf("check\n");
                if (!run_background) {
                    i++;
                } else {
                    i--;
                }
                // printf("parsed [++i] %d\n", i);
                int fd1 = creat(parsed[i], 0644);  // parsed[++i],the file before >
                dup2(fd1, STDOUT_FILENO);          // parsed[++i],closed
                close(fd1);
            }

            // printf("check\n");
            if (run_background) {
                // printf("backkkkk\n");
                // setpgid(0, 0);  // put the child into a new process group.
                // printf(">>> $ \n");
            }
            // if (flag == 2) {  // help, echo...
            //     printf("%s", built_in_str[0]);
            //     fflush(stdout);
            // } else if (flag == 0) {
            if (execvp(parsed[0], parsed) < 0) {  // if redirection it won't exe ??
                printf("\nCould not execute command..");
                exit(0);
            }
            // }
            // else {
            //     printf("check\n");
            // }
            // printf("check\n");}

        }

        else {  //代表為父程序

            if (run_background) {
                printf("[Pid]: %d\n", pid);
                // waitpid(.., .., WNOHANG) // parent不等child process
                // int status;
                // if (waitpid(pid, NULL, WNOHANG) < 0) {
                //     perror("wait error\n");
                //     exit(EXIT_FAILURE);
                // }
                return;
            } else {
                // waiting for child to terminate
                // wait(NULL);
                if (waitpid(pid, NULL, 0) < 0) {
                    perror("wait error\n");
                    exit(EXIT_FAILURE);
                }
                return;
            }
        }
    }
}

// Function where the piped system commands is executed
void execArgsPiped(char** parsed, char** parsedpipe) {  // 1.3 run two-process pipelines
    // 0 is read end, 1 is write end
    int pipefd[2];
    pid_t p1, p2;

    if (pipe(pipefd) < 0) {
        printf("\nPipe could not be initialized");
        return;
    }
    p1 = fork();
    if (p1 < 0) {
        printf("\nCould not fork");
        return;
    }

    if (p1 == 0) {
        // Child 1 executing..
        // It only needs to write at the write end
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);

        if (execvp(parsed[0], parsed) < 0) {
            printf("\nCould not execute command 1..");
            exit(0);
        }
    } else {
        // Parent executing
        p2 = fork();
        close(pipefd[1]);
        if (p2 < 0) {
            printf("\nCould not fork");
            return;
        }

        // Child 2 executing..
        // It only needs to read at the read end
        if (p2 == 0) {
            close(pipefd[1]);
            dup2(pipefd[0], STDIN_FILENO);

            if (execvp(parsedpipe[0], parsedpipe) < 0) {
                printf("\nCould not execute command 2..");
                exit(0);
            }
        } else {
            // parent executing, waiting for two children
            wait(NULL);
            wait(NULL);
        }
    }
}

void execArgsMultiPiped(char** strpiped, char** parsed, int piped) {
    char* built_in_str[1];  // be used to store strings return from builtin

    int fd[2];  // pipe in and out
    pid_t pid, first_child_pid;
    int fdd = 0; /* Backup */
    // long int main_pid = (long)getpid();
    // printf("main_pid = %ld\n", main_pid);

    // check redirection_mode: 0 no, 1 = <, 2 = >, 3 = <+>
    int i;  // get how many cmds
    int redirection_mode = 0, run_background = 0, run_background_pipe = 0;

    // use this to know
    char* backup[MAXLIST];            // not to destroy the last strpipe
    strcpy(backup, strpiped[piped]);  // backup last strpipe
    parseSpace(backup, parsed);       // strpipe 0 | strpipe 1 | ...
    i = redirection_background_check(parsed, &redirection_mode, &run_background_pipe);

    int ii = 0;                        // iith strpipe start from 0
    parseSpace(strpiped[ii], parsed);  // strpipe 0 | strpipe 1 | ...
    i = redirection_background_check(parsed, &redirection_mode, &run_background);

    int flag = 1;
    flag = ownCmdHandler(parsed, built_in_str);
    // printf("flag = %d\n", flag);

    while (strpiped[ii] != NULL) {
        pipe(fd); /* Sharing bidiflow */
        int fd0, fd1;

        if ((pid = fork()) == -1) {  // error
            perror("fork");
            exit(1);
        } else if (pid == 0) {  // child

            int flag = 1;
            flag = ownCmdHandler(parsed, built_in_str);

            if (flag == 2) {  // help, echo...
                if (!run_background) {
                    i++;
                } else {
                    i--;
                }

                fflush(stdout);

                close(STDOUT_FILENO);
                close(fd[0]);

                dup(fd[1]);
                printf("%s", built_in_str[0]);
                // dup2(fdd, STDIN_FILENO);
                // dup2(fd[1], STDOUT_FILENO);

                fflush(stdout);
                exit(1);

                // if (strpiped[++ii] != NULL) {    // if have next cmd
                //     dup2(fd[1], STDOUT_FILENO);  // pipe to next's
                // }
                // close(fd[0]);

            } else {
                if (ii == 0) {
                    // first_child_pid = (long)getpid();
                    // printf("first child %ld\n", first_child_pid);
                }
                // redirection
                if (redirection_mode == 1 || redirection_mode == 3) {
                    fd0 = open(parsed[2], O_RDONLY);  // parsed[2],the file after <
                    dup2(fd0, STDIN_FILENO);          // parsed[2],closed
                    close(fd0);
                }

                if (redirection_mode == 2 || redirection_mode == 3) {
                    // printf("check\n");
                    if (!run_background) {
                        i++;
                    } else {
                        i--;
                    }
                    // printf("parsed [++i] %d\n", i);
                    fd1 = creat(parsed[i], 0644);  // parsed[++i],the file before >
                    dup2(fd1, STDOUT_FILENO);      // parsed[++i],closed
                    close(fd1);
                }
                if (run_background_pipe) {
                    // printf("backkkkk\n");
                }
                // redirection
                //
                dup2(fdd, STDIN_FILENO);
                if (strpiped[++ii] != NULL) {    // if have next cmd
                    dup2(fd[1], STDOUT_FILENO);  // pipe to next's
                }
                close(fd[0]);
                //
                execvp(parsed[0], parsed);
                exit(1);
            }

        } else {  // parent
            if (run_background_pipe) {
                printf("[Pid]: %d\n", pid);  // only print first child pid
                run_background_pipe = 0;
            } else {
                // printf("parent pid = %d\n", pid);
                // wait(NULL); /* Collect childs */
                if (waitpid(pid, NULL, 0) < 0) {
                    perror("wait error\n");
                    exit(EXIT_FAILURE);
                }
            }
            close(fd[1]);
            fdd = fd[0];
            ii++;
        }
        if (strpiped[ii] != NULL) {  // prepare for next while parsed
            parseSpace(strpiped[ii], parsed);
            redirection_mode = 0, run_background = 0;
            i = redirection_background_check(parsed, &redirection_mode, &run_background);
        }
    }
}

// Help command builtin
void openHelp(char** built_in_str) {
    // puts(
    //     "\n***WELCOME TO MY LITTLE SHELL HELP***"
    //     "\nType program names and arguments, and hit enter."
    //     "\n"
    //     "\nThe following are build-in:"
    //     "\n1: help\tshow all build-in func info"
    //     "\n2: cd:\tchange directory"
    //     "\n3: echo:\techo the str to std output"
    //     "\n4: record:\tshow last-16 cmds u typed in"
    //     "\n5: replay:\tre-execute the cmd showed in record"
    //     "\n6: mypid:\tfind and print prcess-ids"
    //     "\n7: exit:\texit shell"
    //     "\n"
    //     "\nUse the \"man\" cmd for info on the other programs"
    //     "\n>all other general commands available in UNIX shell"
    //     "\n>pipe handling"
    //     "\n>improper space handling");

    // char* arguments[MAXCOM] = {
    //     "\n***WELCOME TO MY LITTLE SHELL HELP***",
    //     "\nType program names and arguments, and hit enter.",
    //     "\n",
    //     "\nThe following are build-in:",
    //     "\n1: help\tshow all build-in func info",
    //     "\n2: cd:\tchange directory",
    //     "\n3: echo:\techo the str to std output",
    //     "\n4: record:\tshow last-16 cmds u typed in",
    //     "\n5: replay:\tre-execute the cmd showed in record",
    //     "\n6: mypid:\tfind and print prcess-ids",
    //     "\n7: exit:\texit shell",
    //     "\n",
    //     "\n>all other general commands available in UNIX shell",
    //     "\n>pipe handling\n"};

    char* arguments[] = {
        "\n***WELCOME TO MY LITTLE SHELL HELP***\nType program names and arguments, hit enter.\n\nThe following are build-in :\n1 : help\tshow all build-in func info\n2 : cd :\tchange directory\n3 : echo :\techo the str to std output\n4 : record :\tshow last-16 cmds u typed in\n5 : replay :\tre-execute the cmd showed in record\n6 : mypid :\tfind and print prcess-ids\n7 : exit :\texit shell\n\n> all other general commands available in UNIX shell\n> pipe handling \n\n"};

    strcpy(built_in_str, arguments);
    return;
}

// Function to execute builtin commands
int ownCmdHandler(char** parsed, char** built_in_str) {
    int NoOfOwnCmds = 7, i, switchOwnArg = 0;
    char* ListOfOwnCmds[NoOfOwnCmds];
    char* username;

    // ListOfOwnCmds[0] = "exit";
    // ListOfOwnCmds[1] = "cd";
    // ListOfOwnCmds[2] = "help";
    // ListOfOwnCmds[3] = "hello";
    ListOfOwnCmds[0] = "help";    // return 2
    ListOfOwnCmds[1] = "cd";      // return 1
    ListOfOwnCmds[2] = "echo";    // return 2
    ListOfOwnCmds[3] = "record";  // return 2
    ListOfOwnCmds[4] = "replay";  // return 3
    ListOfOwnCmds[5] = "mypid";   // return 2
    ListOfOwnCmds[6] = "exit";

    // printf("strcmp\n");

    for (i = 0; i < NoOfOwnCmds; i++) {
        if (strcmp(parsed[0], ListOfOwnCmds[i]) == 0) {
            switchOwnArg = i + 1;
            break;
        }
    }
    // printf("switchOwnArg %d\n", switchOwnArg);

    switch (switchOwnArg) {
        case 1:  // help
            openHelp(built_in_str);
            return 2;
        case 2:  // cd
            chdir(parsed[1]);
            return 1;
        case 3:;  // echo   // semi-colon is needed
            char command[MAXCOM] = {NULL};
            int i;

            int no_endline = 0;
            if (strcmp(parsed[1], "-n") == 0) {
                no_endline = 1;
                // strcat(command, parsed[0]);
            } else {
                // strcat(command, parsed[0]);  // parsed[0]=echo
                strcat(command, parsed[1]);
                strcat(command, " ");
            }
            for (i = 2; i < MAXLIST; i++) {  // break until parsed[i] == NULL
                if (parsed[i] == NULL) {
                    if (!no_endline) {
                        strcat(command, "\n");
                    }
                    break;
                } else {
                    // printf("%s\n", parsed[i]);
                    // copy str //parsed[0] == echo
                    strcat(command, parsed[i]);  // ex: (echo) aa bb
                    strcat(command, " ");
                }
            }
            // printf("%s", command);  // ex: (echo) aa bb
            *built_in_str = command;
            // printf("%s", built_in_str[0]);

            return 2;
        case 4:;  // record//
            register HIST_ENTRY** the_list;
            register int ii;

            char commandd[MAXCOM] = {NULL};

            char temp[100];

            the_list = history_list();
            if (the_list)
                for (ii = 0; the_list[ii]; ii++) {
                    // printf("%d: %s\n", ii, the_list[ii]->line);
                    int iii = ii + 1;
                    sprintf(temp, "%d: %s\n", iii, the_list[ii]->line);
                    strcat(commandd, temp);
                }
            *built_in_str = commandd;
            // printf("that is\n%s", built_in_str[0]);

            return 2;
        case 5:  // replay//

            return 3;

        case 6:;  // mypid//
            char commanddd[MAXCOM] = {NULL};
            char tempp[100];

            if (!strcmp(parsed[1], "-i")) {
                // printf("pid>%ld, ppid>%ld\n", (long)getpid(), (long)getppid());
                // printf("pid>%ld\n", (long)getpid());
                sprintf(tempp, "pid>%ld\n", (long)getpid());
                strcat(commanddd, tempp);
            } else if (!strcmp(parsed[1], "-p")) {
                // printf("djidjidjdi\n");
                char filename[1000];
                sprintf(filename, "/proc/%s/stat", parsed[2]);
                // printf("%s\n", filename);
                FILE* f = fopen(filename, "r");
                if (f == NULL) {
                    printf("open error\n");
                    // printf("mypid -p: process id not exist\n");
                    return 1;
                }
                // printf("djidjidjdi\n");
                int unused;
                char comm[1000];
                char state;
                int ppid;
                fscanf(f, "%d %s %c %d", &unused, comm, &state, &ppid);
                // printf("comm = %s\n", comm);
                // printf("state = %c\n", state);
                // printf("djidjidjdi\n");
                if (ppid != 0) {
                    // printf("parent pid = %d\n", ppid);
                    sprintf(tempp, "parent pid = %d\n", ppid);
                    strcat(commanddd, tempp);
                } else {
                    printf("mypid -p: process id not exist\n");
                }
                fclose(f);
            } else if (!strcmp(parsed[1], "-c")) {
                char filename[1000];
                sprintf(filename, "/proc/%s/task/%s/children", parsed[2], parsed[2]);
                FILE* f = fopen(filename, "r");
                if (f == NULL) {
                    printf("open error\n");
                    // printf("mypid -c: process id not exist\n");
                    return 1;
                }

                char buff[255];  // creating char array to store data of file

                while (fscanf(f, "%s", buff) != EOF) {
                    // printf("%s\n", buff);
                    sprintf(tempp, "%s\n", buff);
                    strcat(commanddd, tempp);
                }
                fclose(f);
            }
            *built_in_str = commanddd;

            return 2;
        case 7:  // exit
            printf("\nGoodbye!! See you next time.\n");
            exit(0);
        default:
            break;
    }

    return 0;
}

// function for finding pipe
int parsePipe(char* str, char** strpiped) {
    int i;
    for (i = 0; i < MAXLIST; i++) {
        strpiped[i] = strsep(&str, "|");
        if (strpiped[i] == NULL) {
            i--;
            break;
        }
    }

    if (strpiped[1] == NULL)
        return 0;  // returns zero if no pipe is found.
    else {
        return i;
    }
}

// function for parsing command words
void parseSpace(char* str, char** parsed) {
    int i;
    for (i = 0; i < MAXLIST; i++) {
        parsed[i] = strsep(&str, " ");
        // printf("parsed s = %s\n", parsed[i]);
        if (parsed[i] == NULL) {
            break;
        }
        if (strlen(parsed[i]) == 0) {
            i--;
        }
    }
    // for (i = 0; i < 5; i++) {
    //     printf("parsed %d = %s\n", i, parsed[i]);
    // }
    // printf("out\n");
}

void parse_replay(char* str, char** parsed) {
    parsed[0] = strsep(&str, "replay");
    // printf("parsed s = %s\n", parsed[0]);
    strsep(&str, " ");
    parsed[1] = strsep(&str, " ");  // num of replay
    // printf("parsed s = %s\n", parsed[1]);
    parsed[2] = strsep(&str, "");
    // parsed[2] = &str;
    // strcpy(parsed[2], str);
    // printf("parsed s2 = %s\n", parsed[2]);
}

void processString(char* str, char** parsed, char** parsedpipe) {
    char* strpiped[MAXLIST];

    int piped = 0;

    piped = parsePipe(str, strpiped);  // function for finding pipe

    // printf("\npiped = %d\n", piped);
    // printf("strpiped[0] = %s\n", strpiped[0]);
    // printf("strpiped[1] = %s\n", strpiped[1]);

    if (piped == 1) {
        // parseSpace(strpiped[0], parsed);
        // parseSpace(strpiped[1], parsedpipe);
        execArgsMultiPiped(strpiped, parsed, piped);

    } else if (piped > 1) {
        // printf("\npiped = %d\n", piped); //piped > how many "|"
        execArgsMultiPiped(strpiped, parsed, piped);
    } else {  // piped = 0
        // printf("aa\n");
        parseSpace(str, parsed);  // function for parsing command words
        // ownCmdHandler(parsed);
        execArgs(parsed);
        // printf("aaa\n");
    }

    // printf("parsed = %s", parsed);

    // if (ownCmdHandler(parsed))  // if is built-in cmd
    //     return 0;
    // else
    //     return 1 + piped;
}

int main() {
    char inputString[MAXCOM];
    char inputString_copy[MAXCOM];
    char* parsedArgs[MAXLIST];
    char* parsedArgsPiped[MAXLIST];
    int execFlag = 0;
    char* is_replay = "replay";
    char* parsereplay[MAXLIST];

    init_shell();

    using_history();
    stifle_history(Last_16_CMD);  // Stifle the history list, remembering only the last 16 entries.

    while (1) {
        // printf("prompt> ");
        // take input
        memset(inputString, NULL, sizeof(inputString));  //全部歸null
        if (takeInput(inputString)) {
            continue;
        }
        if (strstr(inputString, is_replay) != NULL) {  // if 有replay
            strcpy(inputString_copy, inputString);
            parse_replay(inputString_copy, parsereplay);
            // printf("\n\nparsed s2 ===== %s\n\n", parsereplay[2]);

            register HIST_ENTRY** the_list;
            register int ii;

            char commandd[MAXCOM] = {NULL};

            char temp[100];

            the_list = history_list();

            if (the_list) {
                int num = strtol(parsereplay[1], (char**)NULL, 10);
                num--;
                // printf("num = %d\n", num);
                sprintf(temp, "%s", the_list[num]->line);
                strcat(commandd, temp);
                // printf("cmd = %s\n", commandd);
            }
            // strcpy(parsereplay[1], commandd);
            // printf("\nparsed s0 = %s\n", parsereplay[0]);
            // printf("parsed s1 = %s\n", parsereplay[1]);
            // printf("parsed s2 = %s\n\n", parsereplay[2]);

            char commandddd[MAXCOM] = {NULL};
            strcat(commandddd, parsereplay[0]);
            // const char* const space = " ";
            // char space[2];
            // strcpy(space, " ");
            char* spa = " ";

            strcat(commandddd, commandd);
            strcat(commandddd, spa);
            if (parsereplay[2]) {
                strcat(commandddd, parsereplay[2]);
            }
            // printf("command = %s\n", commandddd);
            strcpy(inputString, commandddd);
            // printf("input = %s\n", inputString);
            add_history(commandd);
        }

        // process
        processString(inputString, parsedArgs, parsedArgsPiped);
    }
    return 0;
}
