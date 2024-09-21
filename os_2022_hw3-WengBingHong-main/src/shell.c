#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../include/builtin.h"
#include "../include/command.h"

int execute(struct pipes *p) {
    for (int i = 0; i < num_builtins(); ++i)
        if (strcmp(p->args[0], builtin_str[i]) == 0)
            return (*builtin_func[i])(p->args);
    return execvp(p->args[0], p->args);
}

int spawn_proc(int in, int out, struct cmd *cmd, struct pipes *p) {
    pid_t pid;
    int status, fd;
    if ((pid = fork()) == 0) {
        if (in != 0) {
            dup2(in, 0);
            close(in);
        } else {
            if (cmd->in_file) {
                fd = open(cmd->in_file, O_RDONLY);
                dup2(fd, 0);
                close(fd);
            }
        }
        if (out != 1) {
            dup2(out, 1);
            close(out);
        } else {
            if (cmd->out_file) {
                fd = open(cmd->out_file, O_RDWR | O_CREAT, 0644);
                dup2(fd, 1);
                close(fd);
            }
        }
        if (execute(p) == -1)
            perror("lsh");
        exit(EXIT_FAILURE);
    } else {
        if (cmd->background) {
            if (!p->next)
                printf("[pid]: %d\n", pid);
        } else {
            do {
                waitpid(pid, &status, WUNTRACED);
            } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        }
    }
    return 1;
}

int fork_pipes(struct cmd *cmd) {
    int in = 0, fd[2];
    struct pipes *temp = cmd->head;
    while (temp->next != NULL) {
        pipe(fd);
        spawn_proc(in, fd[1], cmd, temp);
        close(fd[1]);
        in = fd[0];
        temp = temp->next;
    }
    if (in != 0) {
        spawn_proc(in, 1, cmd, temp);
        return 1;
    }

    spawn_proc(0, 1, cmd, cmd->head);
    return 1;
}

void shell() {
    while (1) {
        printf(">>> $ ");

        char *buffer = read_line();
        if (buffer == NULL)
            continue;

        struct cmd *cmd = split_line(buffer);

        int status = -1;
        if (!cmd->background && cmd->head->next == NULL) {
            int fd, in = dup(0), out = dup(1);
            if (cmd->in_file) {
                fd = open(cmd->in_file, O_RDONLY);
                dup2(fd, 0);
                close(fd);
            }
            if (cmd->out_file) {
                fd = open(cmd->out_file, O_RDWR | O_CREAT, 0644);
                dup2(fd, 1);
                close(fd);
            }
            for (int i = 0; i < num_builtins(); ++i)
                if (strcmp(cmd->head->args[0], builtin_str[i]) == 0)
                    status = (*builtin_func[i])(cmd->head->args);
            if (cmd->in_file) dup2(in, 0);
            if (cmd->out_file) dup2(out, 1);
            close(in);
            close(out);
        }
        if (status == -1)
            status = fork_pipes(cmd);

        while (cmd->head) {
            struct pipes *temp = cmd->head;
            cmd->head = cmd->head->next;
            free(temp->args);
            free(temp);
        }
        free(cmd);
        free(buffer);

        if (status == 0)
            break;
    }
}
