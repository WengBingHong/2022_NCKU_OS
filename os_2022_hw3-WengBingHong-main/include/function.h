#ifndef FUNCTION_H
#define FUNCTION_H

void test_exit();
void test_sleep();
void test_resource1();
void test_resource2();
void idle();

void task1();
void task2();
void task3();
void task4();
void task5();
void task6();
void task7();
void task8();
void task9();

extern const char *function_str[];

extern const void (*function_func[])(void);

extern int num_functions();

#endif
