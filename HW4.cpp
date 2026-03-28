#include <iostream>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <random>

std::mt19937 rng(std::random_device{}());

class Simulation {
    private:
        // simulation parameters
        int MAX_PROCESSES;
        double avg_arrival_rate;
        double avg_service_time;
        int scheduling_policy;      // 0 for FCFS, 1 for SJF

        // for reporting purposes
        double accumulated_turnaround_time = 0.0;
        double accumulated_waiting_time = 0.0;
        double accumulated_busy_time = 0.0;

        double avg_turnaround_time;
        double total_throughput;
        double cpu_utilization;
        double avg_processes_waiting;

        // state variables and simulation management variables
        double clock;
        int processes_count;
        bool server_busy;
        // ready_queue and event_queue are vectors of Event structs, which will 
        // be used to manage the events in the simulation
        struct Event {
            int id;
            double time;
            bool isArrival;
            double arrival_time;
            double service_time;
        };    
        std::vector<Event> ready_queue;     // sends processes to our server
        std::vector<Event> event_queue;     // manages all events in the simulation
        void generate_initial_event() {
            Event e;
            e.id = 0;
            e.time = 0;
            e.arrival_time = 0;
            e.isArrival = true;
            e.service_time = exponential_random(avg_service_time);
            event_queue.push_back(e);
        }

        double exponential_random(double lambda) {
            std::uniform_real_distribution<double> dist(0.0, 1.0);
            double random_number = dist(rng);
            double result = -1 * log(random_number) / lambda;
            return result;
        }

        void handle_arrival(Event *e) {
            if (!server_busy) {
                server_busy = true;
                
                // Edit event e to be a departure event then schedule e in 
                // correct place in event queue based on condition
                // Also update accumulated waiting time for reporting purposes
                e->arrival_time = clock;
                e->time = clock + e->service_time;
                e->isArrival = false;
                schedule_event(e);
            } else {
                place_event_in_ready_queue(e);
            }
            
            // generate a random poisson variable to determine the time of  
            // the next arrival event and update e's time accordingly
            Event new_e;
            new_e.id = e->id + 1;
            new_e.time = clock + exponential_random(avg_arrival_rate);
            new_e.arrival_time = new_e.time;
            new_e.isArrival = true;
            new_e.service_time = exponential_random(avg_service_time);
            schedule_event(&new_e);
        }
        
        void handle_departure(Event *e) {
            if (!ready_queue.empty()) {
                Event incoming_e = ready_queue.front();
                // Edit incoming event e to be a departure event then schedule
                // e in correct place
                ready_queue.erase(ready_queue.begin());
                incoming_e.time = clock + incoming_e.service_time;
                incoming_e.isArrival = false;
                accumulated_waiting_time += clock - incoming_e.arrival_time;
                schedule_event(&incoming_e);
            } else {
                server_busy = false;
                // reporting purposes
                accumulated_turnaround_time += clock - e->arrival_time;
                accumulated_busy_time += e->service_time;
            }
            processes_count++;
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
                   double service_time, 
                   int policy)
            : MAX_PROCESSES(max_procs) {
            // Initialize simulation parameters
            avg_arrival_rate = arrival_rate;
            avg_service_time = service_time;
            scheduling_policy = policy;
            clock = 0;
            processes_count = 0;
            server_busy = false;
        }

        void run() {
            generate_initial_event();
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

        void output_results() {
            avg_turnaround_time = accumulated_turnaround_time / processes_count;
            total_throughput = processes_count / clock;
            cpu_utilization = accumulated_busy_time / clock;
            avg_processes_waiting = (accumulated_waiting_time / processes_count) * avg_arrival_rate;

            std::cout << avg_turnaround_time << std::endl;
            std::cout << total_throughput << std::endl;
            std::cout << cpu_utilization << std::endl;
            std::cout << avg_processes_waiting << std::endl;
        }
};



int main(int argc, char *argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] 
                  << " <avg_arrival_rate> <avg_service_time> <scheduling_policy>"
                  << std::endl;
        return 1;
    }
    const int PROCESS_LIMIT = 10000;

    // input parameters for the simulation (entered by user)
    // ***NOTE: there is no error handling for user input yet***
    double avg_arrival_rate, 
           avg_service_time;
    int scheduling_policy;      // 0 for FCFS, 1 for SJF

    avg_arrival_rate = std::atof(argv[1]);
    avg_service_time = 1 / std::atof(argv[2]);
    scheduling_policy = std::atoi(argv[3]);

    Simulation sim(PROCESS_LIMIT, 
                    avg_arrival_rate, 
                    avg_service_time, 
                    scheduling_policy);
    sim.run();
    sim.output_results();
    return 0;
}