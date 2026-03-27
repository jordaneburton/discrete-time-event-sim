#include <iostream>
#include <vector>

struct Process {
    /* data */
    int id;
    int arrival_time;
    bool is_arriving;
};


void run_simulation(const int MAX_PROCESSES) {
    int clock = 0;
    int processes_count = 0;

    bool server_busy = false;
    std::vector<Process> ready_queue;
    // Simulation code goes here
}

int main() {
    const int PROCESS_LIMIT = 10000;
    run_simulation(PROCESS_LIMIT);

    return 0;
}