#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <unistd.h>
#include <sys/wait.h>

using namespace std;

struct Process {
    int pid;
    int burst_time;
    int priority;
    int remaining_time;
    int waiting_time;
};

class Scheduler {
private:
    vector<Process> processes;
    int quantum;

public:
    //Task 1: Initialize Processes
    void initializeProcesses() {
        int n;
        cout << "Enter the number of processes: ";
        cin >> n;

        for (int i = 0; i < n; ++i) {
            pid_t child_pid = fork();
            if (child_pid == 0) {
                // Child process
                cout << "Process created with PID: " << getpid() << endl;
                exit(0);
            } else if (child_pid > 0) {
                // Parent process
                Process p;
                p.pid = child_pid;
                cout << "Enter burst time for process " << i + 1 << ": ";
                cin >> p.burst_time;
                cout << "Enter priority for process " << i + 1 << ": ";
                cin >> p.priority;
                p.remaining_time = p.burst_time;
                p.waiting_time = 0;
                processes.push_back(p);
            } else {
                cerr << "Fork failed\n";
                exit(1);
            }
        }

        // Wait for all child processes to finish
        for (int i = 0; i < n; ++i) {
            wait(NULL);
        }

        cout << n << " processes created with unique PIDs printed.\n\n";
    }
    
    //Task 2: User Interface for Scheduler Configuration
    void configureScheduler() {
        int choice;
        cout << "Scheduling algorithm:\n";
        cout << "1. Round-Robin\n";
        cout << "2. Priority-based\n";
        cin >> choice;

        if (choice == 1) {
            cout << "Enter time quantum for RR Scheduling: ";
            cin >> quantum;
            cout << "Round-Robin selected. Quantum time set to " << quantum << ".\n";
        } else {
            cout << "Priority-based scheduling selected.\n";
        }
        cout << endl;
    }
    
    //Task 3: Implementing Round-Robin Scheduling\n
    void roundRobinScheduling() {
        queue<Process*> readyQueue;
        for (auto& process : processes) {
            readyQueue.push(&process);
        }

        while (!readyQueue.empty()) {
            Process* current_process = readyQueue.front();
            readyQueue.pop();

            int execution_time = min(quantum, current_process->remaining_time);
            current_process->remaining_time -= execution_time;

            cout << "Process " << current_process->pid << " executed for " << execution_time << " units.\n";

            if (current_process->remaining_time > 0) {
                readyQueue.push(current_process);
            }

            cout << "Remaining burst times: ";
            for (const auto& p : processes) {
                cout << p.remaining_time << " ";
            }
            cout << endl;
        }
        cout << endl;
    }
    //Task4-Implementing Priority Based Scheduling
    void priorityBasedScheduling() {
        sort(processes.begin(), processes.end(),
             [](const Process& a, const Process& b) { return a.priority < b.priority; });

        for (const auto& process : processes) {
            cout << "Process " << process.pid << " executed (priority " << process.priority << ").\n";
        }
        cout << endl;
    }
    
    //Task 5: Calculating Average Waiting Time
    double calculateAverageWaitingTime() {
        int total_waiting_time = 0;
        int current_time = 0;
        for (auto& process : processes) {
            process.waiting_time = current_time;
            total_waiting_time += process.waiting_time;
            current_time += process.burst_time;
        }
        double avg_waiting_time = static_cast<double>(total_waiting_time) / processes.size();
        cout << "Average Waiting Time: " << avg_waiting_time << endl << endl;
        return avg_waiting_time;
    }
    
    //Task 6: Comparing and Evaluating Performance
    void compareAndEvaluate() {
        double rr_avg_waiting_time = calculateAverageWaitingTime();
       
        // Resetting processes for priority-based scheduling
        for (auto& process : processes) {
            process.remaining_time = process.burst_time;
            process.waiting_time = 0;
        }

        priorityBasedScheduling();
        double priority_avg_waiting_time = calculateAverageWaitingTime();

        cout << "Round-Robin Average Waiting Time: " << rr_avg_waiting_time << endl;
        cout << "Priority-based Average Waiting Time: " << priority_avg_waiting_time << endl;

        if (rr_avg_waiting_time < priority_avg_waiting_time) {
            cout << "Round-Robin performed better with lower average waiting time.\n";
        } else if (rr_avg_waiting_time > priority_avg_waiting_time) {
            cout << "Priority-based scheduling performed better with lower average waiting time.\n";
        } else {
            cout << "Both algorithms performed equally in terms of average waiting time.\n";
        }
    }
};

int main() {
    Scheduler scheduler;
    scheduler.initializeProcesses();
    scheduler.configureScheduler();
    scheduler.roundRobinScheduling();
    scheduler.compareAndEvaluate();
    return 0;
}
