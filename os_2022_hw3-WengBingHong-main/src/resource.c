#include "../include/resource.h"

#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "../include/builtin.h"

// There should be 8 resources with id 0-7 in the simulation.
// How to simulate resources is up to your design.
// For example, you can use a boolean array,
// resource_available = { true, false, true, .... }

#define handle_error(msg)   \
    do {                    \
        perror(msg);        \
        exit(EXIT_FAILURE); \
    } while (0)

void get_resources(int count, int *resources) {
    // get old context
    ready_q_head->my_task.uctx_func_old.uc_stack.ss_sp = ready_q_head->my_task.func_stack_old;
    ready_q_head->my_task.uctx_func_old.uc_stack.ss_size = sizeof(ready_q_head->my_task.func_stack_old);
    ready_q_head->my_task.uctx_func_old.uc_link = &uctx_main;
    if (getcontext(&ready_q_head->my_task.uctx_func_old) == -1)
        handle_error("getcontext");
    // store old context

    // Check if all resources in the list are available
    bool Y = 1;  // yes or no
    for (int i = 0; i < count; i++) {
        if (!resource_available[resources[i]]) {
            Y = 0;
            break;
        }
    }
    if (Y) {
        ready_q_head->my_task.uctx_func_old.uc_stack.ss_size = 0;  // del old context
        for (int i = 0; i < 8; i++) {
            if (i < count) {
                ready_q_head->my_task.resource_occupied[i] = resources[i];  // resources occupied
                printf("Task %s gets resource %d.\n", ready_q_head->my_task.name, resources[i]);
                resource_available[resources[i]] = 0;
            } else {
                ready_q_head->my_task.resource_occupied[i] = -1;
            }
            ready_q_head->my_task.resource_needed[i] = -1;
        }
    } else {
        for (int i = 0; i < 8; i++) {
            if (i < count) {
                ready_q_head->my_task.resource_needed[i] = resources[i];  // resources needed
            } else {
                ready_q_head->my_task.resource_needed[i] = -1;
            }
        }
        printf("Task %s is waiting resource.\n", ready_q_head->my_task.name);
        // printf("sending SIGHUP no resource\n");
        raise(SIGHUP);
        // This task will be switched to WAITING state
        // When all resources in the list are available,
        // this task will be switched to READY state
        // Check again when CPU is dispatched to this task
    }
}

void release_resources(int count, int *resources) {
    // ready_q_head->my_task.uctx_func_old.uc_stack.ss_size = 0;  // remove old context
    for (int i = 0; i < count; i++) {
        printf("Task %s releases resource %d.\n", ready_q_head->my_task.name, resources[i]);
        resource_available[resources[i]] = 1;
        ready_q_head->my_task.resource_occupied[i] = -1;
    }
}
