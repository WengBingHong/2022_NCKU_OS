#include "../include/builtin.h"

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ucontext.h>
#include <unistd.h>

#include "../include/command.h"
#include "../include/function.h"

// builtin functions //
void set_dispatch_algo(char *args);
int help(char **args);
int cd(char **args);
int echo(char **args);
int record(char **args);
bool isnum(char *str);
int mypid(char **args);
void append_task_q_list(struct my_task input);
int add(char **args);
int del(char **args);
int ps(char **args);
int start(char **args);
const char *builtin_str[];
const int (*builtin_func[])(char **);
int num_builtins();
#define IDLE 4

// sort linked list
struct Node *SortedMerge(struct Node *front, struct Node *back);
void FrontBackSplit(struct Node *source, struct Node **frontRef, struct Node **backRef);
void ready_q_sort();

// signal handler
#define SIGTERM 15
#define SIGTSTP 20
bool usr_pause = 0;
void SIGTERM_handler(int sig);
void SIGHUP_handler(int sig);
void SIGVTALRM_handler(int sig);
void SIGTSTP_handler(int sig);
void time_resource_handle();
__sighandler_t my_signal(int sig, __sighandler_t handler);

void dispatch_cpu_idle();
void undispatch_cpu_idle();
void dispatch_first_ready();

// tasks //
// linked list node
struct Node *task_q_head = NULL, *task_q_tail = NULL, *ready_q_head = NULL, *waiting_q_head = NULL;
struct my_task CPU_Idle;

int resource_available[8] = {1, 1, 1, 1, 1, 1, 1, 1};

#define handle_error(msg)   \
    do {                    \
        perror(msg);        \
        exit(EXIT_FAILURE); \
    } while (0)

ucontext_t uctx_main;

long long vt_task_time = 0;

#define time_quantum 3
int time_quantum_left = time_quantum;
int last_time_quantum_left = 0;  // for rr

int algo;  // FCFS 0, RR 1, PP (priority-based preemptive) 2
#define FCFS 0
#define RR 1
#define PP 2

void set_dispatch_algo(char *args) {
    if (!strcmp(args, "FCFS"))
        algo = FCFS;
    else if (!strcmp(args, "RR"))
        algo = RR;
    else if (!strcmp(args, "PP"))
        algo = PP;
    else
        algo = -1;
}

int help(char **args) {
    int i;
    printf("--------------------------------------------------\n");
    printf("My Little Shell!!\n");
    printf("The following are built in:\n");
    for (i = 0; i < num_builtins(); i++) {
        printf("%d: %s\n", i, builtin_str[i]);
    }
    printf("%d: replay\n", i);
    printf("--------------------------------------------------\n");
    return 1;
}

int cd(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "lsh: expected argument to \"cd\"\n");
    } else {
        if (chdir(args[1]) != 0)
            perror("lsh");
    }
    return 1;
}

int echo(char **args) {
    bool newline = true;
    for (int i = 1; args[i]; ++i) {
        if (i == 1 && strcmp(args[i], "-n") == 0) {
            newline = false;
            continue;
        }
        printf("%s", args[i]);
        if (args[i + 1])
            printf(" ");
    }
    if (newline)
        printf("\n");

    return 1;
}

int exit_shell(char **args) {
    return 0;
}

int record(char **args) {
    if (history_count < MAX_RECORD_NUM) {
        for (int i = 0; i < history_count; ++i)
            printf("%2d: %s\n", i + 1, history[i]);
    } else {
        for (int i = history_count % MAX_RECORD_NUM; i < history_count % MAX_RECORD_NUM + MAX_RECORD_NUM; ++i)
            printf("%2d: %s\n", i - history_count % MAX_RECORD_NUM + 1, history[i % MAX_RECORD_NUM]);
    }
    return 1;
}

bool isnum(char *str) {
    for (int i = 0; i < strlen(str); ++i) {
        if (str[i] >= 48 && str[i] <= 57)
            continue;
        else
            return false;
    }
    return true;
}

int mypid(char **args) {
    char fname[BUF_SIZE];
    char buffer[BUF_SIZE];
    if (strcmp(args[1], "-i") == 0) {
        pid_t pid = getpid();
        printf("%d\n", pid);

    } else if (strcmp(args[1], "-p") == 0) {
        if (args[2] == NULL) {
            printf("mypid -p: too few argument\n");
            return 1;
        }

        sprintf(fname, "/proc/%s/stat", args[2]);
        int fd = open(fname, O_RDONLY);
        if (fd == -1) {
            printf("mypid -p: process id not exist\n");
            return 1;
        }

        read(fd, buffer, BUF_SIZE);
        strtok(buffer, " ");
        strtok(NULL, " ");
        strtok(NULL, " ");
        char *s_ppid = strtok(NULL, " ");
        int ppid = strtol(s_ppid, NULL, 10);
        printf("%d\n", ppid);

        close(fd);

    } else if (strcmp(args[1], "-c") == 0) {
        if (args[2] == NULL) {
            printf("mypid -c: too few argument\n");
            return 1;
        }

        DIR *dirp;
        if ((dirp = opendir("/proc/")) == NULL) {
            printf("open directory error!\n");
            return 1;
        }

        struct dirent *direntp;
        while ((direntp = readdir(dirp)) != NULL) {
            if (!isnum(direntp->d_name)) {
                continue;
            } else {
                sprintf(fname, "/proc/%s/stat", direntp->d_name);
                int fd = open(fname, O_RDONLY);
                if (fd == -1) {
                    printf("mypid -p: process id not exist\n");
                    return 1;
                }

                read(fd, buffer, BUF_SIZE);
                strtok(buffer, " ");
                strtok(NULL, " ");
                strtok(NULL, " ");
                char *s_ppid = strtok(NULL, " ");
                if (strcmp(s_ppid, args[2]) == 0)
                    printf("%s\n", direntp->d_name);

                close(fd);
            }
        }

        closedir(dirp);

    } else {
        printf("wrong type! Please type again!\n");
    }

    return 1;
}

void append_task_q_list(struct my_task input) {  // append to the end of the list
    /* allocate node */
    struct Node *new_node = (struct Node *)malloc(sizeof(struct Node));  // cast is not necessarily needed
    if (new_node == NULL)
        handle_error("malloc error");

    /* put in the data */
    new_node->my_task = input;

    /* This new node is last node */
    new_node->task_next = NULL;
    new_node->ready_next = NULL;
    new_node->wait_next = NULL;

    /* If the Linked List is empty, then make the new node as head */
    if (task_q_head == NULL) {
        task_q_head = new_node;
        return;
    }

    /* Else traverse till the last node */
    struct Node *last = task_q_head;
    while (last->task_next != NULL)
        last = last->task_next;

    last->task_next = new_node;
    return;
}

int add(char **args) {  // ex // add task1 abc 3

    // Command format: add {task_name} {function_name} {priority}
    char *task_name = args[1];
    char *function_name = args[2];
    int priority;
    if (algo == PP)  // PP
        priority = atoi(args[3]);
    else
        priority = INT_MIN;

    // Create a task named task_name that runs a function named function_name
    // priority is ignored if the scheduling algorithm is not priority-based preemptive scheduling
    // This task should be set as READY state

    // init
    struct my_task input;

    input.uctx_func.uc_stack.ss_size = 0;
    input.uctx_func_old.uc_stack.ss_size = 0;

    if (task_q_head == NULL) {
        input.TID = 1;
    } else {
        /* traverse till the last node */
        struct Node *last = task_q_head;
        while (last->task_next != NULL)
            last = last->task_next;
        input.TID = last->my_task.TID + 1;
    }
    strcpy(input.name, task_name);
    strcpy(input.func_name, function_name);
    input.state = READY_STATE;
    input.running_time = 0;
    input.waiting_time = 0;
    input.turnaround_time = -1;
    input.priority = priority;

    for (int i = 0; i < 8; i++) {
        input.resource_occupied[i] = -1;
        input.resource_needed[i] = -1;
    }

    input.sleep_time = 0;

    append_task_q_list(input);  // append to the end of the list

    // Print a message in the format: Task {task_name} is ready.
    printf("Task %s is ready.\n", task_name);

    return 1;
}

int del(char **args) {  // del {task_name}

    /* traverse till the last node */
    struct Node *current = task_q_head;

    while (current) {
        if (!strcmp(args[1], current->my_task.name)) {
            current->my_task.state = TERMINATED_STATE;
            printf("Task %s is killed.\n", args[1]);  // Task {task_name} is killed.
            return 1;
        }
        current = current->task_next;
    }
    // printf("Can't del, not found");
    return 1;
}

int ps(char **args) {
    printf("%*s%*s%*s%*s%*s%*s%*s%*s", 5, "TID|", 10, "name|", 12, "state|", 9, "running|", 9, "waiting|", 12, "turnaround|", 11, "resources|", 9, "priority");
    // printf("%*s", 7, "sleep");  // for debugging comment it
    printf("\n-----------------------------------------------------------------------------\n");

    /* traverse till the last node */
    struct Node *current = task_q_head;

    while (current) {
        printf("%*d|%*s|", 4, current->my_task.TID, 9, current->my_task.name);  // TID and name

        if (current->my_task.state == READY_STATE) {  // state
            printf("%*s", 11, "READY");
        } else if (current->my_task.state == WAITING_STATE) {
            printf("%*s", 11, "WAITING");
        } else if (current->my_task.state == RUNNING_STATE) {
            printf("%*s", 11, "RUNNING");
        } else if (current->my_task.state == TERMINATED_STATE) {
            printf("%*s", 11, "TERMINATED");
        }

        printf("|%*d|%*d|", 8, current->my_task.running_time, 8, current->my_task.waiting_time);
        if (current->my_task.turnaround_time == -1) {
            printf("%*s|", 11, "none");
        } else {
            printf("%*d|", 11, current->my_task.turnaround_time);
        }

        if (current->my_task.resource_occupied[0] == -1) {
            printf("%*s", 10, "none");
        } else {
            char dest[18] = {0};
            for (int i = 0; i < 8; i++) {
                if (current->my_task.resource_occupied[i] == -1)
                    break;
                char add[3];
                sprintf(add, " %d", current->my_task.resource_occupied[i]);
                strcat(dest, add);
            }
            printf("%*s", 10, dest);
        }

        if (algo == PP) {  // PP
            printf("|%*d", 9, current->my_task.priority);
        } else {
            printf("|%*s", 9, "ignored");
        }
        // printf("|%*d", 6, current->my_task.sleep_time);  // for debugging comment it
        printf("\n");
        current = current->task_next;
    }
    printf("\n");

    // // print linked list data
    // printf("resource : ");
    // for (int i = 0; i < 8; i++) {
    //     printf("%d ", resource_available[i]);
    // }
    // printf("\n");
    // current = task_q_head;
    // while (current) {
    //     printf("TID = %d ", current->my_task.TID);
    //     printf("ready->next = %d, wait->next = %d",
    //            ((current->ready_next == NULL) ? 0 : current->ready_next->my_task.TID),
    //            ((current->wait_next == NULL) ? 0 : current->wait_next->my_task.TID));

    //     printf(" needed : ");
    //     for (int i = 0; i < 8; i++) {
    //         printf("%d ", current->my_task.resource_needed[i]);
    //     }
    //     printf("\n");
    //     current = current->task_next;
    // }
    // // print task queue
    // printf("Task Q:    ");
    // current = task_q_head;
    // while (current) {
    //     printf("%d > ", current->my_task.TID);
    //     current = current->task_next;
    // }
    // printf("NULL\n");

    // // print ready queue
    // printf("Ready Q:   ");
    // current = ready_q_head;
    // while (current) {
    //     printf("%d > ", current->my_task.TID);
    //     current = current->ready_next;
    // }
    // printf("NULL\n");

    // // print waiting queue
    // printf("Waiting Q: ");
    // current = waiting_q_head;
    // while (current) {
    //     printf("%d > ", current->my_task.TID);
    //     current = current->wait_next;
    // }
    // printf("NULL\n\n");

    return 1;
}

int start(char **args) {  // Start or resume the simulation // cpu dispatcher // make context
    if (usr_pause) {
        if (ready_q_head) {
            ready_q_head->my_task.state = READY_STATE;
        }
        ready_q_sort();
    }
    usr_pause = 0;
    printf("Start simulation.\n");

    // ps(NULL);  // comment it

    // set signal handler
    if (my_signal(SIGTERM, SIGTERM_handler) == SIG_ERR ||
        my_signal(SIGVTALRM, SIGVTALRM_handler) == SIG_ERR ||
        my_signal(SIGHUP, SIGHUP_handler) == SIG_ERR ||
        my_signal(SIGTSTP, SIGTSTP_handler) == SIG_ERR)
        handle_error("signal error");

    // setitimer
    struct timeval tv_interval = {0, 10000};
    struct timeval tv_value = {0, 10000};
    struct itimerval it;
    it.it_interval = tv_interval;
    it.it_value = tv_value;
    setitimer(ITIMER_VIRTUAL, &it, NULL);

    ready_q_sort();  // make and sort ready queue
    // loop until ready queue and waiting queue are empty
    while (ready_q_head || waiting_q_head) {  // loop if ready queue or waiting queue have tasks

        if (usr_pause)
            break;

        if (ready_q_head) {
            ready_q_sort();  // make and sort ready queue
            if (ready_q_head->my_task.state == READY_STATE) {
                // printf("dispatch_first_ready\n");
                dispatch_first_ready();
            }
        } else {
            dispatch_cpu_idle();
        }
    }

    if (!usr_pause) {
        // printf("vt_task_time = %lld\n", vt_task_time);  // test, comment it
        // printf("main: exiting\n");                      // test, comment it
        // ps(NULL);                                       // comment it

        printf("Simulation over.\n");
    }

    return 1;
}

const char *builtin_str[] = {
    "help",
    "cd",
    "echo",
    "exit",
    "record",
    "mypid",
    "add",
    "del",
    "ps",
    "start"};

const int (*builtin_func[])(char **) = {
    &help,
    &cd,
    &echo,
    &exit_shell,
    &record,
    &mypid,
    &add,
    &del,
    &ps,
    &start};

int num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
}

struct Node *SortedMerge(struct Node *front, struct Node *back) {
    struct Node *result = NULL;

    /* Base cases */
    if (!front)
        return (back);
    else if (!back)
        return (front);

    /* recur */
    if (front->my_task.priority <= back->my_task.priority) {
        result = front;
        result->ready_next = SortedMerge(front->ready_next, back);
    } else {
        result = back;
        result->ready_next = SortedMerge(front, back->ready_next);
    }
    return result;
}

void FrontBackSplit(struct Node *source, struct Node **frontRef, struct Node **backRef) {
    struct Node *mid, *end;
    struct Node;
    mid = source;
    end = source->ready_next;

    while (end) {
        end = end->ready_next;
        if (end) {
            mid = mid->ready_next;
            end = end->ready_next;
        }
    }
    *frontRef = source;
    *backRef = mid->ready_next;
    mid->ready_next = NULL;
}
void MergeSort(struct Node **headRef) {
    struct Node *head = *headRef;
    struct Node *front = NULL;
    struct Node *back = NULL;

    /* Base case */
    if (!head || !head->ready_next) {
        return;
    }

    FrontBackSplit(head, &front, &back);

    /* Recursively sort */
    MergeSort(&front);
    MergeSort(&back);

    /* answer = merge the two sorted lists together */
    *headRef = SortedMerge(front, back);
}

void ready_q_sort() {
    struct Node *current = NULL;
    if (usr_pause) {
        current = task_q_tail->task_next;
        while (current) {
            task_q_tail = current;
            if (current->my_task.state == READY_STATE) {
                struct Node *last = ready_q_head;
                while (last->ready_next)
                    last = last->ready_next;

                last->ready_next = current;
            }
            current = current->task_next;
        }

    } else if (ready_q_head == NULL) {  // if ready_q_head is NULL, means in the begining of start, or task waiting
                                        // put all ready task to ready queue in FCFS order
                                        /* traverse till the last node */
        current = task_q_head;
        while (current) {
            task_q_tail = current;
            if (ready_q_head == NULL && current->my_task.state == READY_STATE) {
                ready_q_head = current;
            } else if (current->my_task.state == READY_STATE) {
                struct Node *last = ready_q_head;
                while (last->ready_next)
                    last = last->ready_next;

                last->ready_next = current;
            }
            current = current->task_next;
        }
    }

    if (!ready_q_head)
        return;

    if (algo == PP) {
        MergeSort(&ready_q_head);  // pass global variable by reference
        // printf("ready queue sorted\n");
    }

    // print ready queue
    // current = ready_q_head;
    // while (current) {
    //     printf("Task > TID = %d, name = %s, priority = %d\n",
    //            current->my_task.TID, current->my_task.name, current->my_task.priority);
    //     current = current->ready_next;
    // }
}

void SIGTERM_handler(int sig) {
    // printf("\n^_^ Catch SIGTERM %d ^_^\n", sig);
    // ready_q_head->my_task.turnaround_time = vt_task_time - ready_q_head->my_task.turnaround_time;  // last turnaround time
    // ready_q_head->my_task.running_time++;  // running time

    // struct Node *temp = ready_q_head;
    // while (temp) {  // wait time
    //     if (temp->my_task.state != TERMINATED_STATE && temp->my_task.state != RUNNING_STATE)
    //         temp->my_task.waiting_time++;
    //     temp = temp->ready_next;
    // }

    // current = waiting_q_head;
    // while (current) {  // wait time
    //     if (current->my_task.state != TERMINATED_STATE && current->my_task.state != RUNNING_STATE)
    //         current->my_task.waiting_time++;
    //     current = current->wait_next;
    // }
    time_resource_handle();
    ready_q_head->my_task.turnaround_time = ready_q_head->my_task.running_time + ready_q_head->my_task.waiting_time;

    // turnaround time = running time + waiting time
    // printf("turnaround time = %d\n", ready_q_head->my_task.turnaround_time);                                          // comment it
    struct Node *current = ready_q_head;
    ready_q_head = ready_q_head->ready_next;
    current->my_task.state = TERMINATED_STATE;
    current->ready_next = NULL;
    // ps(NULL);  // comment it

    // swap_back_to_main
    if (swapcontext(&current->my_task.uctx_func, &uctx_main) == -1)  // uctx_main gained control
        handle_error("swapcontext");
}

void SIGHUP_handler(int sig) {
    // printf("\n^_^ Catch SIGHUP %d ^_^\n", sig);
    // ready_q_head->my_task.running_time++;  // just like exit, execute sleep is too fast, but have to add one

    // struct Node *current = ready_q_head;
    // while (current) {  // wait time
    //     if (current->my_task.state != TERMINATED_STATE && current->my_task.state != RUNNING_STATE)
    //         current->my_task.waiting_time++;
    //     current = current->ready_next;
    // }

    // current = waiting_q_head;
    // while (current) {  // wait time
    //     if (current->my_task.state != TERMINATED_STATE && current->my_task.state != RUNNING_STATE)
    //         current->my_task.waiting_time++;
    //     current = current->wait_next;
    // }

    time_resource_handle();

    struct Node *current = ready_q_head;
    --ready_q_head->my_task.waiting_time;
    ready_q_head = ready_q_head->ready_next;
    current->my_task.state = WAITING_STATE;
    current->ready_next = NULL;

    // ready queue first task add to waiting queue tail
    struct Node **indirect_last = &waiting_q_head;
    while (*indirect_last)
        indirect_last = &((*indirect_last)->wait_next);
    *indirect_last = current;

    // printf("TID %d goes to waiting queue\n", current->my_task.TID);

    // swap_back_to_main
    if (swapcontext(&current->my_task.uctx_func, &uctx_main) == -1)  // uctx_main gained control
        handle_error("swapcontext");
}

// The signal handler should do the followings:
// i. Calculate all task-related time (granularity: 10ms)
// ii. Check if any tasks' state needs to be switched
// iii. Decide whether re-scheduling is needed
void SIGVTALRM_handler(int sig) {
    // printf("\n^_^ Catch SIGVTALRM %d ^_^\n", sig);
    // vt_task_time++;

    // struct Node **current = &waiting_q_head;
    // while (*current) {
    //     if ((*current)->my_task.state != TERMINATED_STATE && (*current)->my_task.state != RUNNING_STATE)  // wait time new add
    //         (*current)->my_task.waiting_time++;                                                           // new add

    //     if ((*current)->my_task.sleep_time > 0) {  // the sleep time will be ms - 1, so that we won't wait another one time granularity
    //         // printf("--\n");                        // test, comment it
    //         (*current)->my_task.sleep_time--;
    //         // ps(NULL);  // comment it
    //         current = &((*current)->wait_next);
    //     } else if ((*current)->my_task.resource_needed[0] != -1) {
    //         // printf("TID %d wait for resource\n", (*current)->my_task.TID);  // test, comment it
    //         bool Y = 1;
    //         for (int i = 0; i < 8; i++) {
    //             if ((*current)->my_task.resource_needed[i] == -1)
    //                 break;
    //             if (resource_available[(*current)->my_task.resource_needed[i]] == 0) {
    //                 Y = 0;
    //                 break;
    //             }
    //         }
    //         if (Y) {
    //             for (int i = 0; i < 8; i++) {
    //                 (*current)->my_task.resource_needed[i] = -1;
    //             }
    //         }

    //         current = &((*current)->wait_next);
    //     } else {
    //         // printf("wait task to ready queue\n");  // test, comment it
    //         // wait queue add to ready queue
    //         struct Node *temp = *current;
    //         *current = (*current)->wait_next;

    //         temp->wait_next = NULL;
    //         // add to ready queue tail
    //         struct Node **last = &ready_q_head;
    //         while (*last)
    //             last = &((*last)->ready_next);
    //         *last = temp;
    //         temp->my_task.state = READY_STATE;

    //         // ps(NULL);  // comment it
    //     }
    // }

    // if (ready_q_head) {
    //     if (ready_q_head->my_task.state == RUNNING_STATE)  // running time
    //         ready_q_head->my_task.running_time++;

    //     struct Node *temp = ready_q_head;
    //     while (temp) {  // wait time
    //         if (temp->my_task.state != TERMINATED_STATE && temp->my_task.state != RUNNING_STATE)
    //             temp->my_task.waiting_time++;
    //         temp = temp->ready_next;
    //     }

    //     if (CPU_Idle.state == RUNNING_STATE)
    //         undispatch_cpu_idle();
    // }

    // time_quantum_left--;
    time_resource_handle();

    if (algo == RR) {
        if (time_quantum_left == 0) {
            // if (CPU_Idle.state == RUNNING_STATE) {
            //     printf("CPU Idle expire time quantum\n");  // test, comment it
            // } else {
            //     printf("TID %d expire time quantum\n", ready_q_head->my_task.TID);  // test, comment it
            // }
            struct Node *head = ready_q_head;
            if (ready_q_head) {  // ready head to ready tail

                ready_q_head = ready_q_head->ready_next;
                struct Node **last = &ready_q_head;
                while (*last)
                    last = &((*last)->ready_next);
                *last = head;
                head->ready_next = NULL;
                head->my_task.state = READY_STATE;
            }

            // ps(NULL);  // comment it
            if (CPU_Idle.state == RUNNING_STATE) {
                // undispatch_cpu_idle();
            } else {                                                          // swap_back_to_main
                if (swapcontext(&head->my_task.uctx_func, &uctx_main) == -1)  // uctx_main gained control
                    handle_error("swapcontext");
            }
        }
    }
}

void SIGTSTP_handler(int sig) {
    // printf("\n^_^ Catch SIGVTALRM(ctrl+z) %d ^_^\n", sig);
    usr_pause = 1;
    last_time_quantum_left = time_quantum_left;
    // printf("vt_task_time = %lld\n", vt_task_time);
    // printf("task q tail TID = %d\n", task_q_tail->my_task.TID);
    // ready_q_head->my_task.state = READY_STATE;
    // swap_back_to_main
    if (swapcontext(&ready_q_head->my_task.uctx_func, &uctx_main) == -1)  // uctx_main gained control
        handle_error("swapcontext");
}

void time_resource_handle() {
    vt_task_time++;

    struct Node **current = &waiting_q_head;
    while (*current) {
        if ((*current)->my_task.state != TERMINATED_STATE && (*current)->my_task.state != RUNNING_STATE)  // wait time new add
        {
            (*current)->my_task.waiting_time++;
            // printf("%s waiting time = %d\n", (*current)->my_task.name, (*current)->my_task.waiting_time);
        }

        if ((*current)->my_task.sleep_time > 0) {  // the sleep time will be ms - 1, so that we won't wait another one time granularity
            // printf("--\n");                        // test, comment it
            (*current)->my_task.sleep_time--;
            // ps(NULL);  // comment it
            current = &((*current)->wait_next);
        } else if ((*current)->my_task.resource_needed[0] != -1) {
            // printf("TID %d wait for resource\n", (*current)->my_task.TID);  // test, comment it
            bool Y = 1;
            for (int i = 0; i < 8; i++) {
                if ((*current)->my_task.resource_needed[i] == -1)
                    break;
                if (resource_available[(*current)->my_task.resource_needed[i]] == 0) {
                    Y = 0;
                    break;
                }
            }
            if (Y) {
                for (int i = 0; i < 8; i++) {
                    (*current)->my_task.resource_needed[i] = -1;
                }
            }

            current = &((*current)->wait_next);
        } else {
            // printf("wait task to ready queue\n");  // test, comment it
            // wait queue add to ready queue
            struct Node *temp = *current;
            *current = (*current)->wait_next;

            temp->wait_next = NULL;
            // add to ready queue tail
            struct Node **last = &ready_q_head;
            while (*last)
                last = &((*last)->ready_next);
            *last = temp;
            temp->my_task.state = READY_STATE;

            // ps(NULL);  // comment it
        }
    }

    if (ready_q_head) {
        if (ready_q_head->my_task.state == RUNNING_STATE)  // running time
            ready_q_head->my_task.running_time++;

        struct Node *temp = ready_q_head;
        while (temp) {  // wait time
            if (temp->my_task.state != TERMINATED_STATE && temp->my_task.state != RUNNING_STATE) {
                temp->my_task.waiting_time++;
                // printf("%s waiting time = %d\n", temp->my_task.name, temp->my_task.waiting_time);
            }
            temp = temp->ready_next;
        }

        if (CPU_Idle.state == RUNNING_STATE)
            undispatch_cpu_idle();
    }

    time_quantum_left--;
    // ps(NULL);  // comment it
}

__sighandler_t my_signal(int sig, __sighandler_t handler) {
    struct sigaction act;
    struct sigaction oldact;
    act.sa_handler = handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;

    if (sigaction(sig, &act, &oldact) < 0)
        return SIG_ERR;

    return oldact.sa_handler;
}

void dispatch_first_ready() {  // make context and swap
    if (!ready_q_head)         // if ready queue is NULL
        return;
    if (last_time_quantum_left != 0) {
        time_quantum_left = last_time_quantum_left;
        last_time_quantum_left = 0;
    } else {
        time_quantum_left = time_quantum;
    }

    // printf("turnaround time = %d\n", ready_q_head->my_task.turnaround_time);  // comment it

    if (ready_q_head->my_task.uctx_func.uc_stack.ss_size != sizeof(ready_q_head->my_task.func_stack)) {  // if have not maked yet
        if (getcontext(&ready_q_head->my_task.uctx_func) == -1)
            handle_error("getcontext");

        ready_q_head->my_task.uctx_func.uc_stack.ss_sp = ready_q_head->my_task.func_stack;
        ready_q_head->my_task.uctx_func.uc_stack.ss_size = sizeof(ready_q_head->my_task.func_stack);
        ready_q_head->my_task.uctx_func.uc_link = &uctx_main;

        int func_num;
        for (int k = 0; k < num_functions(); k++) {
            if (!strcmp(function_str[k], ready_q_head->my_task.func_name))
                func_num = k;
        }

        makecontext(&ready_q_head->my_task.uctx_func, (*function_func[func_num]), 0);

        // printf("\nmake context\n");
        // printf("swapcontext(&uctx_main, &ready_q_head->my_task.uctx_func)\n");  // comment it
        // printf("Task TID = %d, name = %s, priority = %d\n\n",
        //    ready_q_head->my_task.TID, ready_q_head->my_task.func_name, ready_q_head->my_task.priority);  // comment it
        // Once the scheduler dispatches CPU to a task, print a message in the format:
        printf("Task %s is running.\n", ready_q_head->my_task.name);

        ready_q_head->my_task.state = RUNNING_STATE;
        if (swapcontext(&uctx_main, &ready_q_head->my_task.uctx_func) == -1)  // ready_q_head gained control
            handle_error("swapcontext");
    } else if (ready_q_head->my_task.uctx_func_old.uc_stack.ss_size == sizeof(ready_q_head->my_task.func_stack_old)) {
        // ready_q_head->my_task.uctx_func_old.uc_link = &uctx_main;
        // printf("Task %s is resume.\n", ready_q_head->my_task.name);
        ready_q_head->my_task.state = RUNNING_STATE;
        if (swapcontext(&uctx_main, &ready_q_head->my_task.uctx_func_old) == -1)  // ready_q_head gained control
            handle_error("swapcontext");
    } else {
        // printf("Task %s is maked.\n", ready_q_head->my_task.name);
        ready_q_head->my_task.state = RUNNING_STATE;
        if (swapcontext(&uctx_main, &ready_q_head->my_task.uctx_func) == -1)  // ready_q_head gained control
            handle_error("swapcontext");
    }
}

void dispatch_cpu_idle() {
    // If there are no tasks to be scheduled, but there are still tasks waiting, print a message in the format:
    printf("CPU idle.\n");

    // if (last_time_quantum_left != 0) {
    //     time_quantum_left = last_time_quantum_left;
    //     last_time_quantum_left = 0;
    // } else {
    //     time_quantum_left = time_quantum; ////////////////等助教
    // }

    if (getcontext(&CPU_Idle.uctx_func) == -1)
        handle_error("getcontext");

    CPU_Idle.uctx_func.uc_stack.ss_sp = CPU_Idle.func_stack;
    CPU_Idle.uctx_func.uc_stack.ss_size = sizeof(CPU_Idle.func_stack);
    CPU_Idle.uctx_func.uc_link = &uctx_main;

    CPU_Idle.state = RUNNING_STATE;

    makecontext(&CPU_Idle.uctx_func, (*function_func[IDLE]), 0);  // 4 is idle function
    // printf("\nmakecontext(&CPU_Idle.uctx_func, (*function_func[IDLE]), 0);\n");
    if (swapcontext(&uctx_main, &CPU_Idle.uctx_func) == -1)  // CPU_Idle gained control
        handle_error("swapcontext");
}

void undispatch_cpu_idle() {
    // printf("CPU not idle.\n");
    CPU_Idle.state = TERMINATED_STATE;

    if (swapcontext(&CPU_Idle.uctx_func, &uctx_main) == -1)  // uctx_main gained control
        handle_error("swapcontext");
}