#include <iostream>
#include <vector>


class Simulation {
    private:
        int clock;
        int processes_count;
        const int MAX_PROCESSES;

        bool server_busy;
        std::vector<Process> ready_queue;
        struct Process {
            /* data */
            int id;
            int arrival_time;
            bool is_arriving;
        };    

        void handle_arrival(Process *process) {
            // Handle arrival event
        }
        
        void handle_departure(Process *process) {
            // Handle departure event
        }

    public:
        Simulation(const int max_procs) : MAX_PROCESSES(max_procs) {
            // Initialize simulation parameters
            clock = 0;
            processes_count = 0;
            server_busy = false;
        }

        void run() {
            // Run the simulation
        }
};



int main() {
    const int PROCESS_LIMIT = 10000;
    Simulation sim(PROCESS_LIMIT);
    sim.run();

    return 0;
}