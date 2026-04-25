#include <iostream>
#include <vector>
#include <deque>
#include <cstdlib>
#include <cmath>
#include <random>

std::mt19937 rng(42);  // Fixed seed for reproducibility

class Simulation {
    private:
        // simulation parameters
        int MAX_PROCESSES;
        double avg_arrival_rate;
        double avg_service_time;
        int scheduling_policy;      // 0 for FCFS, 1 for SJF

        // Process structure to track timing metadata
        struct Process {
            int id;
            double arrival_time;
            double service_time;
            double start_time;
            double finish_time;
            
            double turnaround_time() const {
                return finish_time - arrival_time;
            }
            
            bool operator<(const Process& other) const {
                return service_time < other.service_time;
            }
        };

        // Event structure
        struct Event {
            double time;
            int event_type;  // 0 = ARRIVAL, 1 = DEPARTURE
            Process process;
            
            bool operator<(const Event& other) const {
                return time < other.time;
            }
        };

        // Metrics accumulators
        double accumulated_turnaround_time = 0.0;
        double accumulated_busy_time = 0.0;
        double ready_queue_area = 0.0;
        double last_event_time = 0.0;

        // State variables
        double clock = 0.0;
        int completed_count = 0;
        bool server_busy = false;
        Process* current_process = nullptr;
        
        std::vector<Event> event_queue;
        std::deque<Process> fcfs_queue;
        std::vector<Process> sjf_queue;
        int next_pid = 0;

        double exponential_random(double lambda) {
            std::uniform_real_distribution<double> dist(0.0, 1.0);
            double random_number = dist(rng);
            return -1.0 * log(random_number) / lambda;
        }

        void insert_event(const Event& e) {
            auto it = event_queue.begin();
            while (it != event_queue.end() && it->time <= e.time) {
                ++it;
            }
            event_queue.insert(it, e);
        }

        void enqueue_ready(const Process& proc) {
            if (scheduling_policy == 0) {  // FCFS
                fcfs_queue.push_back(proc);
            } else {  // SJF
                auto it = sjf_queue.begin();
                while (it != sjf_queue.end() && it->service_time <= proc.service_time) {
                    ++it;
                }
                sjf_queue.insert(it, proc);
            }
        }

        Process dequeue_ready() {
            Process proc;
            if (scheduling_policy == 0) {  // FCFS
                proc = fcfs_queue.front();
                fcfs_queue.pop_front();
            } else {  // SJF
                proc = sjf_queue.front();
                sjf_queue.erase(sjf_queue.begin());
            }
            return proc;
        }

        int ready_queue_len() const {
            if (scheduling_policy == 0) {
                return fcfs_queue.size();
            } else {
                return sjf_queue.size();
            }
        }

        void handle_arrival(const Process& proc) {
            // Schedule next arrival
            double inter_arrival = exponential_random(avg_arrival_rate);
            double next_arrival_time = clock + inter_arrival;
            Process next_proc;
            next_proc.id = next_pid++;
            next_proc.arrival_time = next_arrival_time;
            next_proc.service_time = exponential_random(1.0 / avg_service_time);
            next_proc.start_time = 0;
            next_proc.finish_time = 0;
            
            Event next_arrival_event;
            next_arrival_event.time = next_arrival_time;
            next_arrival_event.event_type = 0;  // ARRIVAL
            next_arrival_event.process = next_proc;
            insert_event(next_arrival_event);

            // Handle current process
            if (!server_busy) {
                // CPU is free, start serving immediately
                server_busy = true;
                current_process = new Process(proc);
                current_process->start_time = clock;
                
                Event departure_event;
                departure_event.time = clock + current_process->service_time;
                departure_event.event_type = 1;  // DEPARTURE
                departure_event.process = *current_process;
                insert_event(departure_event);
            } else {
                // CPU is busy, put in ready queue
                enqueue_ready(proc);
            }
        }

        void handle_departure(const Process& proc) {
            Process finished_proc = proc;
            finished_proc.finish_time = clock;
            accumulated_turnaround_time += finished_proc.turnaround_time();
            accumulated_busy_time += finished_proc.service_time;
            completed_count++;
            
            if (ready_queue_len() > 0) {
                // Serve next process from ready queue
                Process next_proc = dequeue_ready();
                next_proc.start_time = clock;
                current_process = new Process(next_proc);
                
                Event departure_event;
                departure_event.time = clock + next_proc.service_time;
                departure_event.event_type = 1;  // DEPARTURE
                departure_event.process = next_proc;
                insert_event(departure_event);
            } else {
                // No more processes waiting, CPU becomes idle
                server_busy = false;
                if (current_process != nullptr) {
                    delete current_process;
                    current_process = nullptr;
                }
            }
        }

    public:

        Simulation(const int max_procs, 
                   double arrival_rate, 
                   double service_time, 
                   int policy)
            : MAX_PROCESSES(max_procs),
              avg_arrival_rate(arrival_rate),
              avg_service_time(service_time),
              scheduling_policy(policy) {
        }

        void run() {
            // Schedule first arrival
            double inter_arrival = exponential_random(avg_arrival_rate);
            Process first_proc;
            first_proc.id = next_pid++;
            first_proc.arrival_time = inter_arrival;
            first_proc.service_time = exponential_random(1.0 / avg_service_time);
            first_proc.start_time = 0;
            first_proc.finish_time = 0;
            
            Event first_event;
            first_event.time = inter_arrival;
            first_event.event_type = 0;  // ARRIVAL
            first_event.process = first_proc;
            insert_event(first_event);
            
            clock = inter_arrival;
            last_event_time = 0.0;

            // Main event loop
            while (completed_count < MAX_PROCESSES && !event_queue.empty()) {
                Event event = event_queue.front();
                event_queue.erase(event_queue.begin());

                // Update ready queue area before advancing clock
                double dt = event.time - last_event_time;
                ready_queue_area += ready_queue_len() * dt;
                last_event_time = event.time;
                clock = event.time;

                if (event.event_type == 0) {  // ARRIVAL
                    handle_arrival(event.process);
                } else {  // DEPARTURE
                    handle_departure(event.process);
                }
            }
        }

        void output_results() {
            double avg_turnaround = accumulated_turnaround_time / completed_count;
            double throughput = completed_count / clock;
            double cpu_utilization = accumulated_busy_time / clock;
            double avg_ready_queue = ready_queue_area / clock;

            std::cout << avg_turnaround << std::endl;
            std::cout << throughput << std::endl;
            std::cout << cpu_utilization << std::endl;
            std::cout << avg_ready_queue << std::endl;
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
    double avg_arrival_rate, 
           avg_service_time;
    int scheduling_policy;      // 0 for FCFS, 1 for SJF

    avg_arrival_rate = std::atof(argv[1]);
    avg_service_time = std::atof(argv[2]);  // Fixed: was incorrectly inverting the service time
    scheduling_policy = std::atoi(argv[3]);

    Simulation sim(PROCESS_LIMIT, 
                    avg_arrival_rate, 
                    avg_service_time, 
                    scheduling_policy);
    sim.run();
    sim.output_results();
    return 0;
}