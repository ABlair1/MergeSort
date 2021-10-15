/*
Author: Andrew Blair
Course: CS 344
Date: 8-11-2021
Assignment: Multi-Threaded Merge Sort (Thread and Process Variations)
*/
#define _POSIX_SOURCE 
#define _GNU_SOURCE 
#define _POSIX_C_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <errno.h>
#include <assert.h>

// Global variable declarations
int task_num;
char input_line[100];
char *input_token;
int **in_pipe_array;
int **out_pipe_array;

/*
input_parse:

Function parses the input file line-by-line, converts all alphabetical 
characters to lowercase, and replaces non-alphabetical characters with 
delimiting spaces. Words parsed from lines are placed in pipes to be 
sorted. 
*/
void *input_parse() {
    int pipe_n = 0;
    
    // Open all pipe input streams
    FILE **pipe_in_stream = malloc(task_num * sizeof(FILE*));
    for (int i = 0; i < task_num; i++) {
        pipe_in_stream[i] = fdopen(in_pipe_array[i][1], "w");
		if (pipe_in_stream[i] == NULL) {
            perror("fdopen failed during input parsing\n");
            fprintf(stderr, "Piping #%d failed to write: [Errnum = %d, errmsg = %s]\n",i, errno, strerror(errno));
            exit(EXIT_FAILURE);
		}
    }

    // Get input line-by-line from input file
    while (fgets(input_line, 100, stdin)) {

    // Convert letters to lowercase and replace other chars with spaces
        for (int i = 0; i < strlen(input_line); i++) {
            // If char is uppercase letter, replace with lowercase letter
            if ((int)input_line[i] >= 65 && (int)input_line[i] <= 90) {
                input_line[i] = (char)((int)input_line[i] + 32);
            }

            // If char is non-alphabetical, replace with space
            if ((int)input_line[i] < 97 || (int)input_line[i] > 122) {
                input_line[i] = ' ';
            }
        }

        // Get tokens from line and send "round robin" to file buffers
        input_token = strtok(input_line, " ");
        while (input_token) {
            fputs(input_token, pipe_in_stream[pipe_n]);
            fputs("\n", pipe_in_stream[pipe_n]);
            if (pipe_n == (task_num - 1)) {
                pipe_n = 0;
            } else {
                pipe_n++;
            }
            input_token = strtok(NULL, " ");
        }
    }

    // Close streams after all data has been sent to pipes
    for (int i = 0; i < task_num; i++) {
        if (fclose(pipe_in_stream[i]) != 0) {
            perror("Parser failed to close a pipe stream\n");
            exit(1);
        }
        
    }

    return NULL;
}

/*
input_sort:

Function will spawn a process to sort words in a pipe stream and output 
the sorted words to another pipe stream.
*/
void *input_sort(int pipe_arg) {
    // Get the pipe number the current process will sort
    int pipe_num = pipe_arg;

    // // Close open pipes inherited from main that are not needed here
    // if (close(in_pipe_array[pipe_num][1])) {
    //     perror("Write FD of input pipe not closed in sort process");
    //     exit(1);
    // }
    // if (close(out_pipe_array[pipe_num][0])) {
    //     perror("Read FD of output pipe not closed in sort process");
    //     exit(1);
    // }

    // Redirect I/O for the sort process 
    if (dup2(in_pipe_array[pipe_num][0], STDIN_FILENO) == -1) {
        perror("dup2() failed to redirect stdin\n");
        exit(1);
    }
    if (dup2(out_pipe_array[pipe_num][1], STDOUT_FILENO) == -1) {
        perror("dup2() failed to redirect stdout\n");
        exit(1);
    }

    // // Close open pipe file descriptors following process I/O redirection
    // if (close(in_pipe_array[pipe_num][0])) {
    //     perror("Read FD of input pipe not closed in sort process");
    //     exit(1);
    // }
    // if (close(out_pipe_array[pipe_num][1])) {
    //     perror("Write FD of output pipe not closed in sort process");
    //     exit(1);
    // }

    // Close all pipes prior to exec
    for (int i = 0; i < task_num; i++) {
        for (int j = 0; j < 2; j++) {
            if (close(in_pipe_array[i][j])) {
                perror("Input pipe not closed in sort process\n");
                exit(1);
            }
            if (close(out_pipe_array[i][j])) {
                perror("Output pipe not closed in sort process\n");
                exit(1);
            }
        }
    }

    // Sort the contents of the specified pipe
    execl("/bin/sort", "/bin/sort", STDIN_FILENO, NULL);

    return NULL;
}

/*
merge_results:

Function will dynamically merge sorted lists of words from 
pipe streams and output to stdout. Duplicate words will only be 
ouput once, and a count of occurances will be output on the same 
line as each word. 
*/
void *merge_results() {
    char **compare;
    char current_word[100];
    char next_word[100];
    int next_index;
    int current_count = 0;

    // Open output streams for sorted pipes
    FILE **pipe_out_stream = malloc(task_num * sizeof(FILE*));
    for (int i = 0; i < task_num; i++) {
        pipe_out_stream[i] = fdopen(out_pipe_array[i][0], "r");
        if (pipe_out_stream[i] == NULL) {
            perror("fdopen() failed during merge\n");
            exit(1);
            fflush(stdout);
        }
    }

    // Create array for comparing words from sorted pipe streams
    // Initialize array with the first word from each pipe stream
    compare = malloc(sizeof(char *) * task_num);
    for (int i = 0; i < task_num; i++) {
        compare[i] = malloc(100);
        if (fgets(compare[i], 100, pipe_out_stream[i]) == NULL) {
            if (ferror(pipe_out_stream[i])) {
                printf("Error flag set");
                fflush(stdout);
            }
            strcpy(compare[i], "\0");
        }
    }

    // Get first word that will be written to output
    if (strcmp(compare[0], "\0")) {
        strcpy(next_word, compare[0]);
        next_index = 0;
    } else {
        perror("The input file contains no words\n");
        exit(1);
    }
    for (int i = 1; i < task_num; i++) {
        if (strcmp(compare[i], "\0")) {
            if (strcmp(compare[i], next_word) < 0) {
                strcpy(next_word, compare[i]);
                next_index = i;
            }
        }
    }
    strcpy(current_word, next_word);
    current_count++;

    // Get next word in order
    if (fgets(compare[next_index], 100, pipe_out_stream[next_index]) == NULL) {
        strcpy(compare[next_index], "\0");
    }
    for (int i = 0; i < task_num; i++) {    // Get first available word in array
        if (strcmp(compare[i], "\0")) {
            strcpy(next_word, compare[i]);
            next_index = i;
            break;
        } else if (i == (task_num - 1)) {
            strcpy(next_word, "\0");               // If here, pipes are empty
        }
    }
    if (strcmp(next_word, "\0")) {
        for (int i = 0; i < task_num; i++) {    // Compare words in array
            if (strcmp(compare[i], "\0")) {
                if (strcmp(compare[i], next_word) < 0) {
                    strcpy(next_word, compare[i]);
                    next_index = i;
                }
            }
        }
    }

    // Continue comparing words in array and sorting into output 
    // until all words have been output
    while (strcmp(next_word, "\0") != 0) {

        // If the next word is a duplicate of the current word, increment 
        // count and ignore next word
        if (strcmp(current_word, next_word) == 0) {
            current_count++;
        }
        // If the next word is different from the current, write the current 
        // word and count to output and replace current with next
        else {
            printf("%7d %s", current_count, current_word);
            strcpy(current_word, next_word);
            current_count = 1;
        }

        // Get the next word in order from the comparison array
        if (fgets(compare[next_index], 100, pipe_out_stream[next_index]) == NULL) {
            strcpy(compare[next_index], "\0");
        }
        for (int i = 0; i < task_num; i++) {    // Get first available word in array
            if (strcmp(compare[i], "\0")) {
                strcpy(next_word, compare[i]);
                next_index = i;
                break;
            } else if (i == (task_num - 1)) {
                strcpy(next_word, "\0");               // If here, pipes are empty
            }
        }
        if (strcmp(next_word, "\0")) {
            for (int i = 0; i < task_num; i++) {    // Compare words in array
                if (strcmp(compare[i], "\0")) {
                    if (strcmp(compare[i], next_word) < 0) {
                        strcpy(next_word, compare[i]);
                        next_index = i;
                    }
                }
            }
        }
    }

    // Output last word and count
    printf("%7d %s", current_count, current_word);

    // Close all sorted pipe streams
    for (int i = 0; i < task_num; i++) {
        fclose(pipe_out_stream[i]);
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    // Validate command line args
    if (argc != 2) {
        printf("Usage: msort <number_of_sort_tasks>\n");
        printf("Provide a number of sort tasks greater than 0\n");
        exit(1);
    }
    
    // Get the number of sorting tasks from user command
    if (atoi(argv[1]) > 0) {
        task_num = atoi(argv[1]);
    } else {
        printf("Usage: msort <number_of_sort_tasks>\n");
        printf("Provide a number of sort tasks greater than 0\n");
        exit(1);
    }

    // Create pipes that will sort input
    in_pipe_array = malloc(sizeof(int*) * task_num);
    for (int i = 0; i < task_num; i++) {
        in_pipe_array[i] = malloc(sizeof(int) * 2); 
        if (pipe(in_pipe_array[i])) {
            perror("Input pipe not created");
            exit(1);
        }
    }

    // Create pipes where sorted output will be sent
    out_pipe_array = malloc(sizeof(int*) * task_num);
    for (int i = 0; i < task_num; i++) {
        out_pipe_array[i] = malloc(sizeof(int) * 2); 
        if (pipe(out_pipe_array[i])) {
            perror("Output pipe not created");
            exit(1);
        }
    }

    // Spawn child processes to sort input
    pid_t sorting_pids[task_num];

    for (int i = 0; i < task_num; i++) {
        sorting_pids[i] = fork();
        if (sorting_pids[i] == -1) {
            perror("Sorting child fork() failed");
            exit(1);
        } 
        else if (sorting_pids[i] == 0) {
            input_sort(i);
            exit(1);
        }
    }

    // Close open pipe file descriptors after spawning sorting processes
    for (int i = 0; i < task_num; i++) {
        if (close(in_pipe_array[i][0])) {
            perror("Read FD of input pipe not closed");
            exit(1);
        }
        if (close(out_pipe_array[i][1])) {
            perror("Write FD of output pipe not closed");
            exit(1);
        }
    }

    // Process version of program execution
    #ifdef PROCESS
        // Parse input file for words
        pid_t parser_pid;
        int parser_status;

        parser_pid = fork();
        if (parser_pid == -1) {
            perror("Parser child fork() failed\n");
            exit(1);
        }
        else if (parser_pid == 0) {
            input_parse();
            exit(1);
        }

        // Close pipes left open for the parsing process
        for (int i = 0; i < task_num; i++) {
            if (close(in_pipe_array[i][1])) {
                perror("Write FD of input pipe not closed");
                exit(1);
            }
        }

        // Wait for parsing process to finish
        waitpid(parser_pid, &parser_status, 0);

        // Merge result of sorting processes and output
        pid_t merge_pid;
        int merge_status;

        merge_pid = fork();
        if (merge_pid == -1) {
            perror("Merge child fork() failed\n");
            exit(1);
        }
        else if (merge_pid == 0) {
            merge_results();
            exit(1);
        }

        // Close pipes left open for the merging process
        for (int i = 0; i < task_num; i++) {
            if (close(out_pipe_array[i][0])) {
                perror("Read FD of output pipe not closed");
                exit(1);
            }
        }

        // Wait for merging and output to finish
        waitpid(merge_pid, &merge_status, 0);
    
    // Threaded version of program execution
    #else
        // Parse input file for words
        pthread_t parser_thrd;
        pthread_create(&parser_thrd, NULL, input_parse, NULL);
        
        // Wait for parsing thread to finish
        pthread_join(parser_thrd, NULL);

        // Merge result of sorting processes and output
        pthread_t merge_thrd;
        pthread_create(&merge_thrd, NULL, merge_results, NULL);

        // Wait for merging and output to finish
        pthread_join(merge_thrd, NULL);

    #endif

    return 0;
}