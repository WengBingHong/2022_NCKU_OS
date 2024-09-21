#include "../include/function.h"

#include <stdio.h>
#include <stdlib.h>

#include "../include/resource.h"
#include "../include/task.h"

void test_exit() {
    // printf("test_exit runing\n");  // test, comment it
    // printf("test_exit exit\n");    // test, comment it
    task_exit();

    while (1)
        ;
}

void test_sleep() {
    // printf("test_sleep runing\n");  // test, comment it
    task_sleep(20);
    // printf("test_sleep exit\n");  // test, comment it
    task_exit();

    while (1)
        ;
}

void test_resource1() {
    // printf("test_resource1 runing\n");  // test, comment it
    int resource_list[3] = {1, 3, 7};
    get_resources(3, resource_list);
    task_sleep(5);
    release_resources(3, resource_list);
    // printf("test_resource1 exit\n");  // test, comment it
    task_exit();

    while (1)
        ;
}

void test_resource2() {
    // printf("test_resource2 runing\n");  // test, comment it
    int resource_list[2] = {0, 3};
    get_resources(2, resource_list);
    release_resources(2, resource_list);
    // printf("test_resource2 exit\n");  // test, comment it
    task_exit();

    while (1)
        ;
}

void idle() {
    // printf("idle runing\n");  // test, comment it
    while (1)
        ;
}

void task1() {
    // printf("task1 runing\n");  // test, comment it
    int len = 12000;
    int *arr = (int *)malloc(len * sizeof(int));
    for (int i = 0; i < len; ++i)
        arr[i] = rand();

    for (int i = 0; i < len; ++i) {
        for (int j = i; j < len; ++j) {
            if (arr[i] > arr[j]) {
                arr[i] ^= arr[j];
                arr[j] ^= arr[i];
                arr[i] ^= arr[j];
            }
        }
    }

    free(arr);
    // printf("task1 exit\n");  // test, comment it
    task_exit();

    while (1)
        ;
}

void task2() {
    // printf("task2 runing\n");  // test, comment it
    int size = 512;
    int *matrix[size], *result[size];
    for (int i = 0; i < size; ++i) {
        matrix[i] = (int *)malloc(size * sizeof(int));
        result[i] = (int *)malloc(size * sizeof(int));
    }

    for (int i = 0; i < size; ++i)
        for (int j = 0; j < size; ++j)
            matrix[i][j] = rand() % 100;

    for (int row = 0; row < size; ++row) {
        for (int col = 0; col < size; ++col) {
            result[row][col] = 0;
            for (int i = 0; i < size; ++i) {
                result[row][col] += matrix[row][i] * matrix[i][col];
            }
        }
    }

    for (int i = 0; i < size; ++i) {
        free(matrix[i]);
        free(result[i]);
    }
    // printf("task2 exit\n");  // test, comment it
    task_exit();

    while (1)
        ;
}

void task3() {
    // printf("task3 runing\n");  // test, comment it
    int find_target = 65409;

    int len = 10000000;
    int *list = (int *)malloc(len * sizeof(int));
    for (int i = 0; i < len; ++i)
        list[i] = rand() % find_target;

    for (int i = 0; i < len; ++i)
        if (list[i] == find_target)
            break;

    free(list);
    // printf("task3 exit\n");  // test, comment it
    task_exit();

    while (1)
        ;
}

void task4() {
    // printf("task4 runing\n");  // test, comment it
    int resource_list[3] = {0, 1, 2};
    get_resources(3, resource_list);
    task_sleep(70);  // using resources
    release_resources(3, resource_list);
    // printf("task4 exit\n");  // test, comment it
    task_exit();
    while (1)
        ;
}

void task5() {
    // printf("task5 runing\n");  // test, comment it
    int resource_list_1[2] = {1, 4};
    get_resources(2, resource_list_1);
    task_sleep(20);  // using resources

    int resource_list_2[1] = {5};
    get_resources(1, resource_list_2);
    task_sleep(40);  // using resources

    int resource_list_all[3] = {1, 4, 5};
    release_resources(3, resource_list_all);
    // printf("task5 exit\n");  // test, comment it
    task_exit();
    while (1)
        ;
}

void task6() {
    // printf("task6 runing\n");  // test, comment it
    int resource_list[2] = {2, 4};
    get_resources(2, resource_list);
    task_sleep(60);  // using resources
    release_resources(2, resource_list);
    // printf("task6 exit\n");  // test, comment it
    task_exit();
    while (1)
        ;
}

void task7() {
    // printf("task7 runing\n");  // test, comment it
    int resource_list[3] = {1, 3, 6};
    get_resources(3, resource_list);
    task_sleep(80);  // using resources
    release_resources(3, resource_list);
    // printf("task7 exit\n");  // test, comment it
    task_exit();
    while (1)
        ;
}

void task8() {
    // printf("task8 runing\n");  // test, comment it
    int resource_list[3] = {0, 4, 7};
    get_resources(3, resource_list);
    task_sleep(40);  // using resources
    release_resources(3, resource_list);
    // printf("task8 exit\n");  // test, comment it
    task_exit();
    while (1)
        ;
}

void task9() {
    // printf("task9 runing\n");  // test, comment it
    int resource_list_1[1] = {5};
    get_resources(1, resource_list_1);
    task_sleep(80);  // using resources

    int resource_list_2[2] = {4, 6};
    get_resources(2, resource_list_2);
    task_sleep(40);  // using resources

    int resource_list_all[3] = {4, 5, 6};
    release_resources(3, resource_list_all);
    // printf("task9 exit\n");  // test, comment it
    task_exit();
    while (1)
        ;
}

const char *function_str[] = {
    "test_exit",
    "test_sleep",
    "test_resource1",
    "test_resource2",
    "idle",
    "task1",
    "task2",
    "task3",
    "task4",
    "task5",
    "task6",
    "task7",
    "task8",
    "task9"};

const void (*function_func[])(void) = {
    &test_exit,
    &test_sleep,
    &test_resource1,
    &test_resource2,
    &idle,
    &task1,
    &task2,
    &task3,
    &task4,
    &task5,
    &task6,
    &task7,
    &task8,
    &task9};

int num_functions() {
    return sizeof(function_str) / sizeof(char *);
}
