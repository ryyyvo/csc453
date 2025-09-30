#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_PROCESSES 100
#define MAX_ARGUMENTS 11

int parse_args(int argc, char *argv[], int *quantum, char *processes[MAX_PROCESSES][MAX_ARGUMENTS], int *num_processes)
{
   if (argc < 2)
   {
      fprintf(stderr, "Missing quantum arg\n");
      return 1;
   }

   if ((*quantum = atoi(argv[1])) <= 0)
   {
      fprintf(stderr, "Quantum must be a positive integer\n");
      return 1;
   }
   
   int current_process = 0;
   int current_arg = 0;

   for (int i = 2; i < argc; i++) 
   {
    if (strcmp(argv[i], ":") == 0) {
        if (current_arg > 0) {
            if (current_process + 1 >= MAX_PROCESSES) { 
                fprintf(stderr, "Exceeded maximum number of processes (%d)\n", MAX_PROCESSES);
                return 1;
            }
            current_process++;
            current_arg = 0;
        }
    } 
    else 
    {
        if (current_arg >= MAX_ARGUMENTS) {
            fprintf(stderr, "Exceeded maximum number of arguments per process (%d)\n", MAX_ARGUMENTS);
            return 1;
        }

        processes[current_process][current_arg] = malloc(strlen(argv[i]) + 1);

        if (!processes[current_process][current_arg]) {
            fprintf(stderr, "Memory allocation failed\n");
            return 1;
        }
        strcpy(processes[current_process][current_arg], argv[i]);
        current_arg++;
    }}

    // count the final process if there were arguments
    if (current_arg > 0) {
        current_process++;
    }

   printf("Quantum: %d\n", *quantum);
   *num_processes = current_process;
   return 0;
}

void free_processes(char *processes[MAX_PROCESSES][MAX_ARGUMENTS], int num_processes)
{
    for (int i = 0; i < num_processes; i++) {
        for (int j = 0; j < MAX_ARGUMENTS && processes[i][j] != NULL; j++) {
            free(processes[i][j]);
        }
    }
}

// test 
void print_processes(char *processes[MAX_PROCESSES][MAX_ARGUMENTS], int num_processes)
{
    printf("\nParsed %d processes:\n", num_processes);
    for (int i = 0; i < num_processes; i++) {
        printf("Process %d: ", i);
        for (int j = 0; j < MAX_ARGUMENTS && processes[i][j] != NULL; j++) {
            printf("'%s' ", processes[i][j]);
        }
        printf("\n");
    }
}
// end test


int main(int argc, char *argv[])
{
    int quantum;
    int num_processes = 0;
    char *processes[MAX_PROCESSES][MAX_ARGUMENTS];

    // initialize the processes array to NULL
    memset(processes, 0, sizeof(processes));
    
    if (parse_args(argc, argv, &quantum, processes, &num_processes) != 0) {
        return 1;
    }
    
    print_processes(processes, num_processes);


    // schedule all processes in order
    if (num_processes > 0) {
        for (int proc_idx = 0; proc_idx < num_processes; proc_idx++) {
            printf("\nScheduling process %d: %s\n", proc_idx, processes[proc_idx][0]);
            
            pid_t pid = fork();
            if (pid == 0) {
                // child process
                printf("Executing: %s\n", processes[proc_idx][0]);
                
                execv(processes[proc_idx][0], processes[proc_idx]);
                perror("execv failed");
                exit(1);
            } else if (pid > 0) {
                // parent process 
                int status;
                waitpid(pid, &status, 0);
                printf("Process %d completed with status %d\n", proc_idx, WEXITSTATUS(status));
            } else {
                perror("fork failed");
            }
        }
    } else {
        printf("No processes to schedule.\n");
    }
    
    // clean up memory
    free_processes(processes, num_processes);

   return 0;
}