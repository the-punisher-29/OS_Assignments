//lab's work with arrival time input variation
#include <iostream>
#include <vector>
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>
#include <algorithm>
#include <numeric>

using namespace std;

struct Process {
    int pid;
    int burst_time;
    int priority;
    int arrival_time;
};

// Function for Round-Robin scheduling
float round_robin_scheduling(vector<Process>& processes, int quantum) {
    int n = processes.size();
    vector<int> remaining_bt(n), waiting_time(n), finish_time(n);
    vector<bool> visited(n, false);
    vector<int> queue;
    int completed = 0;
    int time = 0;
    int front = 0;

    fill(remaining_bt.begin(), remaining_bt.end(), 0);
    fill(waiting_time.begin(), waiting_time.end(), 0);
    fill(finish_time.begin(), finish_time.end(), 0);

    for (int i = 0; i < n; ++i) {
        remaining_bt[i] = processes[i].burst_time;
    }

    cout << "\nExecution sequence (Round-Robin):" << endl;

    while (completed < n) {
        for (int i = 0; i < n; ++i) {
            if (processes[i].arrival_time <= time && remaining_bt[i] > 0 && !visited[i]) {
                queue.push_back(i);
                visited[i] = true;
            }
        }

        if (front >= queue.size()) {
            ++time;
            continue;
        }

        int i = queue[front++];

        cout << "Process " << processes[i].pid << " executing from time " << time;

        if (remaining_bt[i] > quantum) {
            time += quantum;
            remaining_bt[i] -= quantum;
        } else {
            time += remaining_bt[i];
            finish_time[i] = time;
            waiting_time[i] = finish_time[i] - processes[i].burst_time - processes[i].arrival_time;
            remaining_bt[i] = 0;
            ++completed;
        }

        cout << " to time " << time << endl;

        for (int j = 0; j < n; ++j) {
            if (processes[j].arrival_time <= time && remaining_bt[j] > 0 && !visited[j]) {
                queue.push_back(j);
                visited[j] = true;
            }
        }

        if (remaining_bt[i] > 0) {
            queue.push_back(i);
        }
    }

    int total_waiting_time = accumulate(waiting_time.begin(), waiting_time.end(), 0);
    float avg_waiting_time = static_cast<float>(total_waiting_time) / n;
    cout << "\nAverage Waiting Time (Round-Robin with Arrival Time): " << avg_waiting_time << endl;

    return avg_waiting_time;
}

// Function for Priority scheduling
float priority_based_scheduling(vector<Process>& processes) {
    int n = processes.size();
    int time = 0;
    int completed = 0;
    vector<int> waiting_time(n), finish_time(n);
    vector<bool> is_completed(n, false);

    cout << "\nExecution sequence (Priority-based):" << endl;

    while (completed < n) {
        int idx = -1;

        for (int i = 0; i < n; ++i) {
            if (processes[i].arrival_time <= time && !is_completed[i]) {
                if (idx == -1 || processes[i].priority < processes[idx].priority) {
                    idx = i;
                }
            }
        }

        if (idx != -1) {
            cout << "Process " << processes[idx].pid << " executing from time " << time;

            time += processes[idx].burst_time;
            finish_time[idx] = time;
            waiting_time[idx] = finish_time[idx] - processes[idx].burst_time - processes[idx].arrival_time;
            is_completed[idx] = true;
            ++completed;

            cout << " to time " << time << " (priority " << processes[idx].priority << ")\n";
        } else {
            ++time;
        }
    }

    int total_waiting_time = accumulate(waiting_time.begin(), waiting_time.end(), 0);
    float avg_waiting_time = static_cast<float>(total_waiting_time) / n;
    cout << "\nAverage Waiting Time (Priority Scheduling with Arrival Time): " << avg_waiting_time << endl;

    return avg_waiting_time;
}

// Function for creating processes
void create_processes(vector<Process>& processes) {
    for (int i = 0; i < processes.size(); ++i) {
        pid_t pid = fork();
        if (pid == 0) {
            processes[i].pid = getpid();
            processes[i].burst_time = rand() % 10 + 1;
            processes[i].priority = rand() % 10 + 1;
            cout << "Process " << i + 1 << " created with PID " << processes[i].pid << endl;
            exit(0);
        } else {
            wait(NULL);
        }
    }
}

int main() {
    //Task1
    int num_processes;
    cout << "Enter the number of processes: ";
    cin >> num_processes;
    vector<Process> processes(num_processes);
    create_processes(processes);
    
    //Task2
    int scheduling_choice, quantum_time = 0;
    cout << "Select Scheduling Algorithm:\n1. Round-Robin\n2. Priority-based\n";
    cin >> scheduling_choice;

    if (scheduling_choice == 1) {
        cout << "\nEnter Quantum Time for Round-Robin: ";
        cin >> quantum_time;
        cout << "\nRound-Robin selected. Quantum time set to " << quantum_time << ".\n";
    } else {
        cout << "Priority-based scheduling selected.\n";
    }

    int n;
    cout << "\nPlease enter the number of processes for Round Robin scheduling: ";
    cin >> n;

    vector<Process> rr_processes(n);
    for (int i = 0; i < n; ++i) {
        cout << "Enter the Burst time of process " << i + 1 << ": ";
        cin >> rr_processes[i].burst_time;
        cout << "Enter the Arrival time of process " << i + 1 << ": ";
        cin >> rr_processes[i].arrival_time;
        rr_processes[i].pid = i + 1;
        rr_processes[i].priority = 0;
    }

    // Task3 and 5-Round Robin Scheduling
    float rr_wait_time = round_robin_scheduling(rr_processes, quantum_time);

    int n_priority;
    cout << "\nPlease enter the number of processes for Priority-based scheduling: ";
    cin >> n_priority;

    vector<Process> priority_processes(n_priority);
    for (int i = 0; i < n_priority; ++i) {
        cout << "Enter the Burst time of process " << i + 1 << ": ";
        cin >> priority_processes[i].burst_time;
        cout << "Enter the Priority of process " << i + 1 << ": ";
        cin >> priority_processes[i].priority;
        cout << "Enter the Arrival time of process " << i + 1 << ": ";
        cin >> priority_processes[i].arrival_time;
        priority_processes[i].pid = i + 1;
    }

    // Task4 and 5-Priority-based Scheduling
    float priority_wait_time = priority_based_scheduling(priority_processes);
    
    

    // Task6-Compare Results
    cout << "\nThe waiting time for the Round Robin Algorithm is: " << rr_wait_time << endl;
    cout << "\nThe waiting time for the Priority-Based Algorithm is: " << priority_wait_time << endl;

    cout << "According to the Waiting time, ";
    if (rr_wait_time > priority_wait_time) {
        cout << "Priority-based performed better than Round Robin\n";
    } else {
        cout << "Round-Robin performed better than Priority-Based\n";
    }

    return 0;
}

