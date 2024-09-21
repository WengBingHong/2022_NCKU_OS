//./scheduler_simulator FCFS
//./scheduler_simulator PP

// #include <stdio.h>
#include <stdlib.h>

#include "include//builtin.h"
#include "include/command.h"
#include "include/shell.h"

int main(int argc, char *argv[]) {
    history_count = 0;
    for (int i = 0; i < MAX_RECORD_NUM; ++i)
        history[i] = (char *)malloc(BUF_SIZE * sizeof(char));

    // printf("Algo = %s\n", argv[1]);
    set_dispatch_algo(argv[1]);

    shell();

    for (int i = 0; i < MAX_RECORD_NUM; ++i)
        free(history[i]);

    return 0;
}
