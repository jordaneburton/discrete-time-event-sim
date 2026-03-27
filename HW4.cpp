#include <iostream>
#include <vector>


class Simulation {
    private:
        int clock;
        int processes_count;
        const int MAX_PROCESSES;

        bool server_busy;
        std::vector<Event> ready_queue;
        struct Event {
            /* data */
            int id;
            int arrival_time;
            bool isArrival;
        };    

        void handle_arrival(Event *e) {
            // Handle arrival event
        }
        
        void handle_departure(Event *e) {
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
            while (processes_count <= MAX_PROCESSES) {
                Event next_event;
                clock = next_event.arrival_time;

                if (next_event.isArrival) {
                    handle_arrival(&next_event);
                } else {
                    handle_departure(&next_event);
                }

                
            }
        }
};



int main() {
    const int PROCESS_LIMIT = 10000;
    Simulation sim(PROCESS_LIMIT);
    sim.run();

    return 0;
}