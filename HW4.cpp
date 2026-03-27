#include <iostream>
#include <vector>


class Simulation {
    private:
        const int MAX_PROCESSES;
        
        double clock;
        int processes_count;
        bool server_busy;
        // ready_queue and event_queue are vectors of Event structs, which will 
        // be used to manage the events in the simulation
        struct Event {
            int id;
            int time;
            bool isArrival;
        };    
        std::vector<Event> ready_queue;     // sends processes to our server
        std::vector<Event> event_queue;     // manages all events in the simulation

        void handle_arrival(Event *e, double service_time) {
            if (!server_busy) {
                server_busy = true;
                
                // Edit event e to be a departure event then schedule e in 
                // correct place in event queue based on condition
                e.id = e->id;
                e.time = clock + service_time;
                e.isArrival = false;
                schedule_event(&e);
            } else {
                ready_queue.push_back(*e);
                // --------------------------------
                // TODO: generate a Poisson random variable to determine the 
                // time of the next arrival event and update e's time accordingly
                // --------------------------------
                e.time = clock + generated_poisson_rv;
                schedule_event(&e);
            }
        }
        
        void handle_departure(Event *e, double finish_time) {
            if (!ready_queue.empty()) {
                e = &ready_queue.front();
                // Edit event e to be a departure event then schedule e in 
                // correct place
                ready_queue.erase(ready_queue.begin());
                e.time = clock + finish_time;
                schedule_event(&e);
            } else {
                server_busy = false;
                // TODO: do reporting
                processes_count++;
            }
        }

        void schedule_event(Event *e) {
            auto it = event_queue.begin();
            while (it != event_queue.end() && it->time <= e->time) {
                ++it;
            }
            event_queue.insert(it, *e);
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
                Event next_event = event_queue.front();
                clock = next_event.time;

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