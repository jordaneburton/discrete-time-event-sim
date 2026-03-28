#include <iostream>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <random>

std::mt19937 rng(std::random_device{}());

class Simulation {
    private:
        int MAX_PROCESSES;
        double avg_arrival_rate;
        double avg_service_rate;
        int scheduling_policy;      // 0 for FCFS, 1 for SJF
        
        double clock;
        int processes_count;
        bool server_busy;
        // ready_queue and event_queue are vectors of Event structs, which will 
        // be used to manage the events in the simulation
        struct Event {
            int id;
            int time;
            bool isArrival;
            double service_time;    // only used for SJF scheduling
        };    
        std::vector<Event> ready_queue;     // sends processes to our server
        std::vector<Event> event_queue;     // manages all events in the simulation
        void generate_initial_event() {
            Event e;
            e.id = 0;
            e.time = 0;
            e.isArrival = true;
            e.service_time = exponential_random(avg_service_rate);
            event_queue.push_back(e);
        }

        double exponential_random(double lambda) {
            std::uniform_real_distribution<double> dist(0.0, 1.0);
            double random_number = dist(rng);
            double result = -1 * log(random_number) / lambda;
            return result;
}

        double generate_arrival_time(double lambda) {
            std::uniform_real_distribution<double> dist(lambda * 0.5, lambda * 1.5);
            double random_lambda = dist(rng);
            return exponential_random(random_lambda);
        }
        void handle_arrival(Event *e) {
            if (!server_busy) {
                server_busy = true;
                
                // Edit event e to be a departure event then schedule e in 
                // correct place in event queue based on condition
                e->time = clock + e->service_time;
                e->isArrival = false;
                schedule_event(e);
            } else {
                place_event_in_ready_queue(e);
                // --------------------------------
                // TODO: generate a Poisson random variable to determine the 
                // time of the next arrival event and update e's time accordingly
                // --------------------------------
                Event new_e;
                new_e.id = e->id + 1;
                new_e.time = clock + generate_arrival_time(avg_arrival_rate);
                new_e.isArrival = true;
                new_e.service_time = exponential_random(avg_service_rate);
                schedule_event(&new_e);
            }
        }
        
        void handle_departure(Event *e) {
            if (!ready_queue.empty()) {
                Event *incoming_e = &ready_queue.front();
                // Edit incoming event e to be a departure event then schedule
                // e in correct place
                ready_queue.erase(ready_queue.begin());
                incoming_e->time = clock + incoming_e->service_time;
                incoming_e->isArrival = false;
                schedule_event(incoming_e);
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

        void place_event_in_ready_queue(Event *e) {
            if (scheduling_policy == 0) {           // FCFS
                ready_queue.push_back(*e);
            } else if (scheduling_policy == 1 ) {   // SJF
                auto it = ready_queue.begin();
                while (it != ready_queue.end() && 
                       it->service_time <= e->service_time) 
                {
                    ++it;
                }
                ready_queue.insert(it, *e);
            }
        }

    public:
        Simulation(const int max_procs, 
                   double arrival_rate, 
                   double service_rate, 
                   int policy)
            : MAX_PROCESSES(max_procs) {
            // Initialize simulation parameters
            avg_arrival_rate = arrival_rate;
            avg_service_rate = service_rate;
            scheduling_policy = policy;
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
                event_queue.erase(event_queue.begin());
            }
        }
};



int main(int argc, char *argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] 
                  << " <avg_arrival_rate> <avg_service_rate> <scheduling_policy>"
                  << std::endl;
        return 1;
    }
    const int PROCESS_LIMIT = 10000;

    // input parameters for the simulation (entered by user)
    // ***NOTE: there is no error handling for user input yet***
    double avg_arrival_rate, 
           avg_service_rate;
    int scheduling_policy;      // 0 for FCFS, 1 for SJF

    avg_arrival_rate = std::atof(argv[1]);
    avg_service_rate = std::atof(argv[2]);
    scheduling_policy = std::atoi(argv[3]);

    Simulation sim(PROCESS_LIMIT, 
                    avg_arrival_rate, 
                    avg_service_rate, 
                    scheduling_policy);
    sim.run();
    
    return 0;
}