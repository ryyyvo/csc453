#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>

#define MAX_PROCESSES 100
#define MAX_ARGUMENTS 11

typedef struct {
    int process_index;
    pid_t pid;          // child pid (0 if not started yet)
    int is_finished;    // 0 = still running, 1 = completed
} ProcessInfo;

int quantum_expired = 0;

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
        if (strcmp(argv[i], ":") == 0)
        {
            if (current_arg > 0)
            {
                if (current_process + 1 >= MAX_PROCESSES)
                {
                    fprintf(stderr, "Exceeded maximum number of processes (%d)\n", MAX_PROCESSES);
                    return 1;
                }
                current_process++;
                current_arg = 0;
            }
        }
        else
        {
            if (current_arg >= MAX_ARGUMENTS)
            {
                fprintf(stderr, "Exceeded maximum number of arguments per process (%d)\n", MAX_ARGUMENTS);
                return 1;
            }

            processes[current_process][current_arg] = malloc(strlen(argv[i]) + 1);

            if (!processes[current_process][current_arg])
            {
                fprintf(stderr, "Memory allocation failed\n");
                return 1;
            }
            strcpy(processes[current_process][current_arg], argv[i]);
            current_arg++;
        }
    }

    // count the final process if there were arguments
    if (current_arg > 0)
    {
        current_process++;
    }

    *num_processes = current_process;
    return 0;
}

void free_processes(char *processes[MAX_PROCESSES][MAX_ARGUMENTS], int num_processes)
{
    for (int i = 0; i < num_processes; i++)
    {
        for (int j = 0; j < MAX_ARGUMENTS && processes[i][j] != NULL; j++)
        {
            free(processes[i][j]);
        }
    }
}

ProcessInfo dequeue(ProcessInfo queue[MAX_PROCESSES], int *queue_size)
{
    ProcessInfo process = queue[0];
    
    // shift all elements forward
    for (int i = 0; i < *queue_size - 1; i++)
    {
        queue[i] = queue[i + 1];
    }
    
    (*queue_size)--;
    return process;
}

void enqueue(ProcessInfo queue[MAX_PROCESSES], int *queue_size, ProcessInfo process)
{   
    queue[*queue_size] = process;
    (*queue_size)++;
}

void timerHandler(int signum)
{
    quantum_expired = 1;
}

int main(int argc, char *argv[])
{
    int quantum;
    int num_processes = 0;
    char *processes[MAX_PROCESSES][MAX_ARGUMENTS];
    struct itimerval timer;

    ProcessInfo queue[MAX_PROCESSES];
    int queue_size = 0;
    pid_t current_pid = 0;
    
    memset(processes, 0, sizeof(processes));
    memset(&timer, 0, sizeof(timer));
    
    signal(SIGALRM, timerHandler);
    
    if (parse_args(argc, argv, &quantum, processes, &num_processes) != 0)
    {
        return 1;
    }
    
    if (num_processes > 0)
    {
        for (int i = 0; i < num_processes; i++)
        {
            queue[i].process_index = i;
            queue[i].pid = 0;  
            queue[i].is_finished = 0;
        }
        queue_size = num_processes;
    }
    
    while (queue_size > 0)
    {
        ProcessInfo current_process = dequeue(queue, &queue_size);
        int proc_idx = current_process.process_index;
       
        if (current_process.pid == 0)
        {
            pid_t pid = fork();
           
            if (pid == 0)
            {
                // child process
                execvp(processes[proc_idx][0], processes[proc_idx]);
                execv(processes[proc_idx][0], processes[proc_idx]);
                perror("execv or execvp failed");
                exit(1);
            }
            else if (pid > 0)
            {
                // save pid 
                current_process.pid = pid;
                current_pid = pid;
            }
            else
            {
                perror("fork failed");
                continue;
            }
        }
        else
        {
            // resume process
            current_pid = current_process.pid;
            kill(current_pid, SIGCONT);
        }
       
        // reset quantum flag
        quantum_expired = 0;
        
        timer.it_value.tv_usec = quantum * 1000;
        timer.it_value.tv_sec = 0;
        timer.it_interval.tv_usec = 0;
        timer.it_interval.tv_sec = 0;
        setitimer(ITIMER_REAL, &timer, NULL);
       
        int status;
        while (!quantum_expired)
        {
            pid_t result = waitpid(current_pid, &status, WNOHANG | WUNTRACED);
           
            if (result == current_pid)
            {
                // check if process status changed
                if (WIFEXITED(status) || WIFSIGNALED(status))
                {
                    current_process.is_finished = 1;
                    break;
                }
            }
           
            usleep(1000); 
        }
       
        // stop if quantum expired and process still running 
        if (!current_process.is_finished && quantum_expired)
        {
            kill(current_pid, SIGSTOP);
           
            enqueue(queue, &queue_size, current_process);
        }
    }
    
    free_processes(processes, num_processes);
    return 0;
}