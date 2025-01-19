
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <sys/wait.h> 

#define MAX_PROCESSES 10
#define TIME_QUANTUM 5 

typedef struct {
    int user_id;
    int weight; 
    int processes[MAX_PROCESSES]; 
    int process_count;
} User;

void print_user_processes(User users[], int user_count) {
    for (int i = 0; i < user_count; i++) {
        printf("User %d (Weight: %d) has %d processes:\n", users[i].user_id, users[i].weight, users[i].process_count);
        for (int j = 0; j < users[i].process_count; j++) {
            printf("  Process %d: %d units\n", j + 1, users[i].processes[j]);
        }
    }
}

void execute_process(int user_id, int process_id, int execution_time, int remaining_time) {
    if (execution_time > 0) {
        printf("User %d: Process %d starts executing for %d units. Remaining after execution: %d units. ", 
               user_id, process_id, execution_time, remaining_time);
        sleep(execution_time / TIME_QUANTUM); 
        if (remaining_time == 0) {
            printf("Process %d finished execution.\n", process_id);
        }
        else{
            printf("\n");
        }
    }
}

void weighted_round_robin(User users[], int user_count) {
    int remaining_processes = 0;

    for (int i = 0; i < user_count; i++) {
        remaining_processes += users[i].process_count;
    }

    while (remaining_processes > 0) {
        for (int i = 0; i < user_count; i++) {
            User *current_user = &users[i];
            int execution_time = current_user->weight * TIME_QUANTUM;

            for (int j = 0; j < current_user->process_count && execution_time > 0; j++) {
                if (current_user->processes[j] > 0) { 
                    int time_to_execute = (current_user->processes[j] <= execution_time) 
                                          ? current_user->processes[j] 
                                          : execution_time;

                    pid_t pid = fork();
                    if (pid == 0) { 
                        execute_process(current_user->user_id, j + 1, time_to_execute, 
                                        current_user->processes[j] - time_to_execute);
                        exit(0);
                    } else if (pid > 0) { 
                        wait(NULL); 
                        if (current_user->processes[j] <= execution_time) {
                            remaining_processes--;
                            execution_time -= current_user->processes[j];
                            current_user->processes[j] = 0;
                        } else {
                            current_user->processes[j] -= execution_time;
                            execution_time = 0;
                        }
                    } else {
                        perror("Fork failed");
                        exit(1);
                    }
                }
            }
        }
    }
}

int main() {
    
    User users[] = {
        {1, 1, {3, 20, 30}, 3}, 
        {2, 2, {15, 25}, 2},    
        {3, 3, {5, 10}, 2}      
    };

    int user_count = sizeof(users) / sizeof(users[0]);

    printf("Initial Processes:\n");
    print_user_processes(users, user_count);

    weighted_round_robin(users, user_count);

    return 0;
}
