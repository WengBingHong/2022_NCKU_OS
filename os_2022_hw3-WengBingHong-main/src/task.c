#include "../include/task.h"

#include <signal.h>
#include <stdio.h>

#include "../include/builtin.h"

#define SIGTERM 15

void task_sleep(int ms) {
    ready_q_head->my_task.sleep_time = ms - 1;  // so that we don't wait another one time granularity
    printf("Task %s goes to sleep.\n", ready_q_head->my_task.name);

    // printf("sending SIGHUP sleep\n");
    raise(SIGHUP);
}

void task_exit() {
    printf("Task %s has terminated.\n", ready_q_head->my_task.name);
    // printf("sending SIGTERM exit\n");
    raise(SIGTERM);
}
