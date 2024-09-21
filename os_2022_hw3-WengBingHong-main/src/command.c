#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "../include/command.h"

char *read_line()
{
    char *buffer = (char *)malloc(BUF_SIZE * sizeof(char));
    if (buffer == NULL) {
        perror("Unable to allocate buffer");
        exit(1);
    }

	if (fgets(buffer, BUF_SIZE, stdin) != NULL) {
		if (buffer[0] == '\n' || buffer[0] == ' ' || buffer[0] == '\t') {
			free(buffer);
			buffer = NULL;
		} else {
			if (strncmp(buffer, "replay", 6) == 0) {
				char *token = strtok(buffer, " ");
				token = strtok(NULL, " ");
				int index = strtol(token, NULL, 10);
				if (index > MAX_RECORD_NUM || index > history_count) {
					free(buffer);
					buffer = NULL;
				} else {
					char *temp = (char *)malloc(BUF_SIZE * sizeof(char));
					int head = 0;
					if (history_count > MAX_RECORD_NUM) {
						head += history_count % MAX_RECORD_NUM;
					}
					strncpy(temp, history[(head + index - 1) % MAX_RECORD_NUM], BUF_SIZE);
					token = strtok(NULL, " ");
					while (token) {
						strcat(temp, " ");
						strcat(temp, token);
						token = strtok(NULL, " ");
					}
					strncpy(buffer, temp, BUF_SIZE);
					free(temp);
					buffer[strcspn(buffer, "\n")] = 0;
					strncpy(history[history_count % MAX_RECORD_NUM], buffer, BUF_SIZE);
					++history_count;
				}
			} else {
				buffer[strcspn(buffer, "\n")] = 0;
				strncpy(history[history_count % MAX_RECORD_NUM], buffer, BUF_SIZE);
				++history_count;
			}
		}
	}

	return buffer;
}

struct cmd *split_line(char *line)
{
	int args_length = 10;
    struct cmd *new_cmd = (struct cmd *)malloc(sizeof(struct cmd));
    new_cmd->head = (struct pipes *)malloc(sizeof(struct pipes));
    new_cmd->head->args = (char **)malloc(args_length * sizeof(char *));
	for (int i = 0; i < args_length; ++i)
		new_cmd->head->args[i] = NULL;
    new_cmd->head->length = 0;
    new_cmd->head->next = NULL;
    new_cmd->background = false;
    new_cmd->in_file = NULL;
    new_cmd->out_file = NULL;

	struct pipes *temp = new_cmd->head;
    char *token = strtok(line, " ");
    while (token != NULL) {
        if (token[0] == '|') {
            struct pipes *new_pipe = (struct pipes *)malloc(sizeof(struct pipes));
			new_pipe->args = (char **)malloc(args_length * sizeof(char *));
			for (int i = 0; i < args_length; ++i)
				new_pipe->args[i] = NULL;
			new_pipe->length = 0;
			new_pipe->next = NULL;
			temp->next = new_pipe;
			temp = new_pipe;
        } else if (token[0] == '<') {
			token = strtok(NULL, " ");
            new_cmd->in_file = token;
        } else if (token[0] == '>') {
			token = strtok(NULL, " ");
            new_cmd->out_file = token;
        } else if (token[0] == '&') {
            new_cmd->background = true;
        } else {
			temp->args[temp->length] = token;
			temp->length++;
        }
        token = strtok(NULL, " ");
    }

    return new_cmd;
}

void test_cmd_struct(struct cmd *cmd)
{
	struct pipes *temp = cmd->head;
	int pipe_count = 0;
	while (temp != NULL) {
		printf("pipe %d: ", pipe_count);
		for (int i = 0; i < temp->length; ++i) {
			printf("%s ", temp->args[i]);
		}
		printf("\n");
		temp = temp->next;
		++pipe_count;
	}
	printf(" in: %s\n", cmd->in_file ? cmd->in_file : "none");
	printf("out: %s\n", cmd->out_file ? cmd->out_file : "none");
	printf("background: %s\n", cmd->background ? "true" : "false");
}
