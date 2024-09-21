#ifndef BUILTIN_H
#define BUILTIN_H

#include <stdbool.h>
#include <ucontext.h>

void set_dispatch_algo(char *args);

int help(char **args);
int cd(char **args);
int echo(char **args);
int exit_shell(char **args);
int record(char **args);
int mypid(char **args);

int add(char **args);
int del(char **args);
int ps(char **args);
int start(char **args);

// void dispatch_array();

// #define MAX_tacks 1024

#define READY_STATE 1
#define WAITING_STATE 2
#define RUNNING_STATE 3
#define TERMINATED_STATE -1

struct my_task {
    ucontext_t uctx_func;
    char func_stack[16384];

    ucontext_t uctx_func_old;
    char func_stack_old[16384];

    // ps data structure
    int TID;  // The TID of each task is unique, and TID starts from 1
    char name[100];
    char func_name[100];
    int state;  // ready 1, waiting 2, running 3, terminated -1

    // Time unit: 10ms
    int running_time;
    int waiting_time;
    int turnaround_time;  // There is no turnaround time for unterminated tasks

    int priority;

    // check in waiting_time
    int resource_occupied[8];
    int resource_needed[8];
    // int *resource;
    int sleep_time;
};

// linked list node
extern struct Node {
    // data
    struct my_task my_task;
    // nodes
    struct Node *task_next;
    struct Node *ready_next;
    struct Node *wait_next;
} *task_q_head, *task_q_tail, *ready_q_head, *waiting_q_head;

extern ucontext_t uctx_main;

// extern int dispatch_count;     // tasks ready number
// extern int swap_count;         // for context switching
// extern int dispatch_arr[][2];  //( num, priority )

extern const char *builtin_str[];

extern const int (*builtin_func[])(char **);

extern int num_builtins();

extern int resource_available[8];

#endif

// #ifndef BUILTIN_H
// #define BUILTIN_H

// #include <ucontext.h>
// #include <stdbool.h>

// void set_dispatch_algo(char *args);

// int help(char **args);
// int cd(char **args);
// int echo(char **args);
// int exit_shell(char **args);
// int record(char **args);
// int mypid(char **args);

// int add(char **args);
// int del(char **args);
// int ps(char **args);
// int start(char **args);

// // void dispatch_array();

// // #define MAX_tacks 1024

// #define READY_STATE 1
// #define WAITING_STATE 2
// #define RUNNING_STATE 3
// #define TERMINATED_STATE -1

// struct my_task {
//     ucontext_t uctx_func;
//     char func_stack[16384];

//     // ps data structure
//     int TID;  // The TID of each task is unique, and TID starts from 1
//     char name[100];
//     char func_name[100];
//     int state;  // ready 1, waiting 2, running 3, terminated -1

//     // Time unit: 10ms
//     int running_time;
//     int waiting_time;
//     int turnaround_time;  // There is no turnaround time for unterminated tasks

//     int priority;

//     // check in waiting_time
//     int resource[8];
//     // int *resource;
//     int sleep_time;

//     // bool context_maked;
// };

// // linked list node
// extern struct Node {
//     // data
//     struct my_task my_task;
//     // nodes
//     struct Node *task_next;
//     struct Node *ready_next;
//     struct Node *wait_next;
// } *task_q_head, *ready_q_head, *waiting_q_head;

// extern bool resource_available[8];

// // extern int dispatch_count;     // tasks ready number
// // extern int swap_count;         // for context switching
// // extern int dispatch_arr[][2];  //( num, priority )

// extern const char *builtin_str[];

// extern const int (*builtin_func[])(char **);

// extern int num_builtins();

// #endif
