#include <iostream>
#include <vector>
#include <deque>
#include <cstdlib>
#include <cmath>
#include <random>

std::mt19937 rng(42);

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
};

// CPU class to encapsulate per-CPU state and behavior
class CPU {
    private:
        int cpu_id;
        bool busy;
        Process* current_process;
        std::deque<Process> ready_queue;
        
    public:
        CPU(int id) : cpu_id(id), busy(false), current_process(nullptr) {}
        
        ~CPU() {
            if (current_process) delete current_process;
        }
        
        bool is_busy() const { return busy; }
        
        void set_busy(bool b) { busy = b; }
        
        void enqueue(const Process& proc) {
            ready_queue.push_back(proc);
        }
        
        Process dequeue() {
            Process proc = ready_queue.front();
            ready_queue.pop_front();
            return proc;
        }
        
        int queue_length() const {
            return ready_queue.size();
        }
        
        bool has_ready_process() const {
            return !ready_queue.empty();
        }
        
        void set_current_process(const Process& proc) {
            current_process = new Process(proc);
        }
        
        Process* get_current_process() { return current_process; }
        
        void clear_current_process() {
            if (current_process) delete current_process;
            current_process = nullptr;
        }
};

class Simulation {
    private:
        // simulation parameters
        int MAX_PROCESSES;
        double avg_arrival_rate;
        double avg_service_time;
        int scenario;               // 1 or 2
        int num_cpus;               // number of CPUs

        // Event structure
        struct Event {
            double time;
            int event_type;  // 0 = ARRIVAL, 1 = DEPARTURE
            int cpu_id;      // which CPU this event is for
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
        std::vector<CPU> cpus;
        std::deque<Process> global_ready_queue;
        
        std::vector<Event> event_queue;
        int next_pid = 0;

        double exponential_random(double lambda) {
            std::uniform_real_distribution<double> dist(0.0, 1.0);
            double random_number = dist(rng);
            return -1.0 * log(random_number) / lambda;
        }

        int select_random_cpu() {
            std::uniform_int_distribution<int> dist(0, num_cpus - 1);
            return dist(rng);
        }

        int find_free_cpu() {
            for (int i = 0; i < num_cpus; ++i) {
                if (!cpus[i].is_busy()) {
                    return i;
                }
            }
            return -1;
        }

        void insert_event(const Event& e) {
            auto it = event_queue.begin();
            while (it != event_queue.end() && it->time <= e.time) {
                ++it;
            }
            event_queue.insert(it, e);
        }

        int total_ready_queue_len() const {
            int total = 0;
            switch (scenario) {
                case 1:
                    // Scenario 1: sum all per-CPU queues
                    for (const auto& cpu : cpus) {
                        total += cpu.queue_length();
                    }
                    break;
                case 2:
                    // Scenario 2: only count global queue
                    total = global_ready_queue.size();
                    break;
                default:
                    break;
            }
            return total;
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
            next_arrival_event.cpu_id = -1;     // not assigned to a CPU yet
            next_arrival_event.process = next_proc;
            insert_event(next_arrival_event);

            switch (scenario) {
                case 1: {
                    // Scenario 1: Select a random CPU, use per-CPU ready queue
                    int selected_cpu = select_random_cpu();
                    CPU& cpu = cpus[selected_cpu];

                    if (!cpu.is_busy()) {
                        // CPU is free, start serving immediately
                        cpu.set_busy(true);
                        cpu.set_current_process(proc);
                        Process* current = cpu.get_current_process();
                        current->start_time = clock;
                        
                        Event departure_event;
                        departure_event.time = clock + current->service_time;
                        departure_event.event_type = 1;  // DEPARTURE
                        departure_event.cpu_id = selected_cpu;
                        departure_event.process = *current;
                        insert_event(departure_event);
                    } else {
                        // CPU is busy, put in its ready queue
                        cpu.enqueue(proc);
                    }
                    break;
                }
                case 2: {
                    // Scenario 2: Find any free CPU or add to global queue
                    int free_cpu = find_free_cpu();
                    
                    if (free_cpu != -1) {
                        // A CPU is free, start serving immediately
                        CPU& cpu = cpus[free_cpu];
                        cpu.set_busy(true);
                        cpu.set_current_process(proc);
                        Process* current = cpu.get_current_process();
                        current->start_time = clock;
                        
                        Event departure_event;
                        departure_event.time = clock + current->service_time;
                        departure_event.event_type = 1;  // DEPARTURE
                        departure_event.cpu_id = free_cpu;
                        departure_event.process = *current;
                        insert_event(departure_event);
                    } else {
                        // All CPUs are busy, add to global queue
                        global_ready_queue.push_back(proc);
                    }
                    break;
                }
                default:
                    break;
            }
        }

        void handle_departure(const Process& proc, int cpu_id) {
            CPU& cpu = cpus[cpu_id];
            
            Process finished_proc = proc;
            finished_proc.finish_time = clock;
            accumulated_turnaround_time += finished_proc.turnaround_time();
            accumulated_busy_time += finished_proc.service_time;
            completed_count++;
            
            switch (scenario) {
                case 1: {
                    // Scenario 1: Use per-CPU ready queue
                    if (cpu.has_ready_process()) {
                        // Serve next process from ready queue
                        Process next_proc = cpu.dequeue();
                        next_proc.start_time = clock;
                        cpu.set_current_process(next_proc);
                        
                        Process* current = cpu.get_current_process();
                        Event departure_event;
                        departure_event.time = clock + current->service_time;
                        departure_event.event_type = 1;  // DEPARTURE
                        departure_event.cpu_id = cpu_id;
                        departure_event.process = *current;
                        insert_event(departure_event);
                    } else {
                        // No more processes waiting, CPU becomes idle
                        cpu.set_busy(false);
                        cpu.clear_current_process();
                    }
                    break;
                }
                case 2: {
                    // Scenario 2: Use global ready queue
                    if (!global_ready_queue.empty()) {
                        // Serve next process from global queue
                        Process next_proc = global_ready_queue.front();
                        global_ready_queue.pop_front();
                        next_proc.start_time = clock;
                        cpu.set_current_process(next_proc);
                        
                        Process* current = cpu.get_current_process();
                        Event departure_event;
                        departure_event.time = clock + current->service_time;
                        departure_event.event_type = 1;  // DEPARTURE
                        departure_event.cpu_id = cpu_id;
                        departure_event.process = *current;
                        insert_event(departure_event);
                    } else {
                        // No more processes waiting, CPU becomes idle
                        cpu.set_busy(false);
                        cpu.clear_current_process();
                    }
                    break;
                }
                default:
                    break;
            }
        }

    public:

        Simulation(const int max_procs, 
                   double arrival_rate, 
                   double service_time, 
                   int scen,
                   int cpus_count)
            : MAX_PROCESSES(max_procs),
              avg_arrival_rate(arrival_rate),
              avg_service_time(service_time),
              scenario(scen),
              num_cpus(cpus_count) {
            // Initialize CPU objects
            for (int i = 0; i < num_cpus; ++i) {
                this->cpus.emplace_back(i);
            }
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
            first_event.cpu_id = -1;     // not assigned to a CPU yet
            first_event.process = first_proc;
            insert_event(first_event);
            
            clock = inter_arrival;
            last_event_time = 0.0;

            while (completed_count < MAX_PROCESSES && !event_queue.empty()) {
                Event event = event_queue.front();
                event_queue.erase(event_queue.begin());

                // Update ready queue area before advancing clock
                double dt = event.time - last_event_time;
                ready_queue_area += total_ready_queue_len() * dt;
                last_event_time = event.time;
                clock = event.time;

                if (event.event_type == 0) {  // ARRIVAL
                    handle_arrival(event.process);
                } else {  // DEPARTURE
                    handle_departure(event.process, event.cpu_id);
                }
            }
        }

        void output_results() {
            double avg_turnaround = accumulated_turnaround_time / completed_count;
            double throughput = completed_count / clock;
            double cpu_utilization = accumulated_busy_time / (num_cpus * clock);
            double avg_ready_queue = ready_queue_area / clock;

            std::cout << avg_turnaround << std::endl;
            std::cout << throughput << std::endl;
            std::cout << cpu_utilization << std::endl;
            std::cout << avg_ready_queue << std::endl;
        }
};



int main(int argc, char *argv[]) {
    if (argc != 5) {
        std::cerr << "Usage: " << argv[0] 
                  << " <avg_arrival_rate> <avg_service_time> <scenario> <num_cpus>"
                  << std::endl;
        return 1;
    }
    const int PROCESS_LIMIT = 10000;

    // input parameters for the simulation (entered by user)
    double avg_arrival_rate, 
           avg_service_time;
    int scenario;               // 1 or 2
    int num_cpus;               // number of CPUs

    avg_arrival_rate = std::atof(argv[1]);
    avg_service_time = std::atof(argv[2]);
    scenario = std::atoi(argv[3]);
    num_cpus = std::atoi(argv[4]);

    Simulation sim(PROCESS_LIMIT, 
                    avg_arrival_rate, 
                    avg_service_time, 
                    scenario,
                    num_cpus);
    sim.run();
    sim.output_results();
    return 0;
}