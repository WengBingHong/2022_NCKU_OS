// gcc -Wall -g -pthread -o test test.c && ./test 2 Test_cases/my_test/read_test_matrix1 Test_cases/my_test/read_test_matrix2
//./MT_matrix 4 Test_cases/Test_case_1/m1.txt Test_cases/Test_case_1/m2.txt //2048*2048
//./MT_matrix 4 Test_cases/Test_case_2/m1.txt Test_cases/Test_case_2/m2.txt //4096*4096
//./MT_matrix 4 Test_cases/Test_case_3/m1.txt Test_cases/Test_case_3/m2.txt //1*4096
//./MT_matrix 4 Test_cases/Test_case_4/m1.txt Test_cases/Test_case_4/m2.txt //4096*1
//./MT_matrix 4 Test_cases/my_test/read_test_matrix1 Test_cases/my_test/read_test_matrix2
#include <assert.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define proc_entry "/proc/thread_info"

// threads shares golbal variables //
int num_of_threads;
int rows[2], cols[2];  // 0 = first matrix, 1 = second matrix
int ffp;

// dynamical Memory Allocation //
int** first_matrix = NULL;
int** second_matrix = NULL;
long long** answer_matrix = NULL;

// add Mutex //
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// functions //
void read_file_rows_cols(char* file_name, int* rows, int* cols);
void read_file_and_store(char* file_name, int matrix_num);
void print_matrix(int matrix_num, int rows, int cols);
void print_answer_matrix();
void write_answer_matrix_to_file();
void calculate_matrix_dispatch_by_element(int Thread_Num);
void* child();  // child thread function
void print_to_console(double elapsed_tim, char* threads, char* file_name);
double Get_time();

int main(int argc, char* argv[]) {
    // get rows and cols from file //
    read_file_rows_cols(argv[2], &rows[0], &cols[0]);
    read_file_rows_cols(argv[3], &rows[1], &cols[1]);
    // printf("1st matrix rows %d cols %d\n", rows[0], cols[0]);
    // printf("2nd matrix rows %d cols %d\n", rows[1], cols[1]);
    //

    // dynamical Memory Allocation //
    first_matrix = (int**)malloc(rows[0] * sizeof(int*));
    for (int i = 0; i < rows[0]; i++) {
        first_matrix[i] = (int*)malloc(cols[0] * sizeof(int));
    }
    // second matrix = row_1*col_1
    second_matrix = (int**)malloc(rows[1] * sizeof(int*));
    for (int i = 0; i < rows[1]; i++) {
        second_matrix[i] = (int*)malloc(cols[1] * sizeof(int));
    }
    // answer matrix = row_0*col_1
    answer_matrix = (long long**)malloc(rows[0] * sizeof(long long*));
    for (int i = 0; i < rows[0]; i++) {
        answer_matrix[i] = (long long*)malloc(cols[1] * sizeof(long long));
    }

    /* // how to free them
    // for (int i = 0; i < rows[0]; i++)
    //     free(first_matrix[i]);
    // free(first_matrix);
    // for (int i = 0; i < rows[1]; i++)
    //     free(second_matrix[i]);
    // free(second_matrix);
    */
    //

    // store 2 matrix //
    read_file_and_store(argv[2], 0);
    read_file_and_store(argv[3], 1);
    //

    // implement pthread to calculate matrix //
    num_of_threads = atoi(argv[1]);  // n threads
    pthread_t t[num_of_threads];     // thread array
    int input[num_of_threads];       // data input for each thread

    for (int i = 0; i < num_of_threads; i++) {
        input[i] = i;
    }

    double start, end;
    double cpu_time_used;

    start = Get_time();
    for (int i = 0; i < num_of_threads; i++) {
        pthread_create(&t[i], NULL, child, (void*)input[i]);  // create child thread
        // printf("%dth thread created\n", i);
    }
    for (int i = 0; i < num_of_threads; i++) {
        pthread_join(t[i], NULL);  // wait child thread to terminate
        // printf("%dth thread terminated\n", i);
    }
    end = Get_time();
    cpu_time_used = end - start;
    // printf("elapse time > %f\n", cpu_time_used);
    //

    // print_answer_matrix();

    write_answer_matrix_to_file();
    print_to_console(cpu_time_used, argv[1], argv[2]);

    exit(EXIT_SUCCESS);
}

void read_file_rows_cols(char* file_name, int* rows, int* cols) {
    FILE* fp;
    char* line = NULL;
    size_t len = 0;
    ssize_t read;

    const char* const delim = " ";
    fp = fopen(file_name, "r");

    if (fp == NULL) {
        printf("no file");
        exit(EXIT_FAILURE);
    }
    if ((read = getline(&line, &len, fp)) != -1) {
        char* const dupstr = strdup(line);
        char* sepstr = dupstr;
        char* substr = NULL;
        substr = strsep(&sepstr, delim);
        *rows = atoi(substr);  // rows
        substr = strsep(&sepstr, delim);
        *cols = atoi(substr);  // cols
        // printf("rows = %d, cols = %d\n", *rows, *cols);
    }
    fclose(fp);
}

void read_file_and_store(char* file_name, int matrix_num) {
    FILE* fp;
    char* line = NULL;
    size_t len = 0;
    ssize_t read;

    const char* const delim = " ";
    int rows = 0, cols = 0;

    fp = fopen(file_name, "r");
    if (fp == NULL) {
        printf("no file");
        exit(EXIT_FAILURE);
    }
    if ((read = getline(&line, &len, fp)) != -1) {  // skip rows and cols
        char* const dupstr = strdup(line);
        char* sepstr = dupstr;
        char* substr = NULL;
        substr = strsep(&sepstr, delim);
        rows = atoi(substr);  // rows
        substr = strsep(&sepstr, delim);
        cols = atoi(substr);  // cols
        // printf("rows = %d, cols = %d\n", rows, cols);
    }
    int count = 0;
    while ((read = getline(&line, &len, fp)) != -1) {
        // printf("Retrieved line of length %zu :\n", read);
        // printf("%s", line);
        char* const dupstr = strdup(line);
        char* sepstr = dupstr;
        char* substr = NULL;
        for (int i = 0; i < cols; i++) {
            substr = strsep(&sepstr, delim);
            // printf("count = %d, i = %d\n", count, i);
            if (matrix_num == 0) {
                first_matrix[count][i] = atoi(substr);

            } else if (matrix_num == 1) {
                second_matrix[count][i] = atoi(substr);
            } else {
                printf("Invalid matrix number");
                exit(EXIT_FAILURE);
            }
            // printf("%s ", substr);  // include "\n" !!
        }
        count++;
    }
    if (line)
        free(line);

    fclose(fp);
}

void print_matrix(int matrix_num, int rows, int cols) {
    if (matrix_num == 0) {
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                printf("%d ", first_matrix[i][j]);
            }
            printf("\n");
        }
    } else if (matrix_num == 1) {
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                printf("%d ", second_matrix[i][j]);
            }
            printf("\n");
        }
    } else {
        printf("Invalid matrix number");
        exit(EXIT_FAILURE);
    }
}

void print_answer_matrix() {
    for (int i = 0; i < rows[0]; i++) {
        for (int j = 0; j < cols[1]; j++) {
            printf("%lld ", answer_matrix[i][j]);
        }
        printf("\n");
    }
}

void write_answer_matrix_to_file() {
    FILE* out_file = fopen("result.txt", "w");  // write only
    if (out_file == NULL) {                     // test for files not existing.
        printf("Error! Could not open file\n");
        exit(-1);  // must include stdlib.h
    }
    fprintf(out_file, "%d %d\n", rows[0], cols[1]);  // write to file
    for (int i = 0; i < rows[0]; i++) {
        for (int j = 0; j < cols[1]; j++) {
            fprintf(out_file, "%lld ", answer_matrix[i][j]);  // write to file
            // printf(        "%lld ", answer_matrix[i][j]);  // write to screen
        }
        fprintf(out_file, "\n");  // write to file
        // printf("\n");      // write to screen
    }
    fclose(out_file);
}

void calculate_matrix_dispatch_by_element(int Thread_Num) {
    int element = Thread_Num;  // get the first element number

    while (element < rows[0] * cols[1]) {  // rows[0] * cols[1] = the elements of answer matrix
        long long temp = 0;
        int row_col[2];
        row_col[0] = element / cols[1];  // rows
        row_col[1] = element % cols[1];  // cols
        // printf("element = %d, rol_cols = (%d,%d)\n", element, row_col[0], row_col[1]);
        for (int i = 0; i < cols[0]; i++) {
            temp += first_matrix[row_col[0]][i] * second_matrix[i][row_col[1]];
        }
        answer_matrix[row_col[0]][row_col[1]] = temp;
        element += num_of_threads;  // go to next element
    }
}

void* child(void* arg) {
    int input = (int*)arg;  // get data
    // printf("input = %d\n", input);

    // ul to str in c //
    unsigned long thread_id = pthread_self();
    const int n = snprintf(NULL, 0, "%lu", thread_id);
    // printf("n = %d\n", n);
    assert(n > 0);
    char buf[n + 1];
    int c = snprintf(buf, n + 1, "%lu", thread_id);
    assert(buf[n] == '\0');
    assert(c == n);
    //

    // printf("pthread ID lu - %lu\n", thread_id);
    // printf("thread gid %d\n", getgid());
    // printf("pthread ID s  - %s\n", buf);

    // calculate_matrix(input[0], input[1]);  // calculate
    calculate_matrix_dispatch_by_element(input);

    // write to proc_entry //
    // right before its termination
    pthread_mutex_lock(&mutex);
    ffp = open(proc_entry, O_APPEND | O_RDWR);
    // printf("write\n");
    write(ffp, buf, sizeof(buf));
    close(ffp);
    pthread_mutex_unlock(&mutex);
    //

    pthread_exit(NULL);
}

void print_to_console(double elapsed_time, char* threads, char* file_name) {
    // printf("User reading...\n");
    printf("PID:%d\n", getpid());
    ffp = open(proc_entry, O_RDWR);

    // FILE* f = fopen("elapse_time.txt", "a");                 // write to file
    // fprintf(f, "\t\t\t%s\n", file_name);                     // write to file
    // fprintf(f, "\t\t\t%s threads:\n", threads);              // write to file
    // fprintf(f, "%f sec > elapse time\n", elapsed_time);      // write to file
    // fprintf(f, "\t\t\t\t\t\t\t\t\t\t\tPID:%d\n", getpid());  // write to file

    for (int i = 0; i < num_of_threads; i++) {
        char time_infos[100];
        read(ffp, time_infos, 100);
        printf("\t%s\n", time_infos);
        // fprintf(f, "\t\t\t\t\t\t\t\t\t\t\t\t%s\n", time_infos);  // write to file
    }
    // fprintf(f, "\n\n");  // write to file

    close(ffp);
    // fclose(f);  // write to file
}

double Get_time() {
    struct timespec ts;
    double sec;
    clock_gettime(CLOCK_REALTIME, &ts);
    sec = ts.tv_nsec;
    sec /= 1e9;
    sec += ts.tv_sec;
    return sec;
}
