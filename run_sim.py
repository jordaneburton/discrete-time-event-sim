import subprocess
import sys
import matplotlib.pyplot as plt

def run_cpp_program(executable_path, args=None):
    command = [executable_path] + (args or [])
    result = subprocess.run(command, capture_output=True, text=True)
    if result.stderr:
        print(result.stderr, end="", file=sys.stderr)
    return result.stdout.splitlines(), result.returncode

if __name__ == "__main__":
    if len(sys.argv) < 4:
        print("Usage: run_sim.py <executable> <avg_service_time> <scenario> <num_cpus>")
        sys.exit(1)

    executable = sys.argv[1]
    service_time = sys.argv[2]
    scenario = sys.argv[3]
    num_cpus = sys.argv[4] if len(sys.argv) > 4 else "1"
    arrival_rates = list(range(10, 31))

    # results[scenario] = list of [turnaround, throughput, utilization, avg_waiting] per arrival rate
    results = {}

    for arrival_rate in arrival_rates:
        if scenario not in results:
            results[scenario] = []
        lines, exit_code = run_cpp_program(executable, [str(arrival_rate), service_time, scenario, num_cpus])
        if exit_code != 0:
            print(f"Error: arrival_rate={arrival_rate}, scenario={scenario}, num_cpus={num_cpus}", file=sys.stderr)
            results[scenario].append([None] * 4)
            continue
        results[scenario].append([float(x) for x in lines])

    labels = ["Avg Turnaround Time", "Total Throughput", "CPU Utilization", "Avg Processes Waiting"]
    scenario_name = f"Scenario {scenario} ({num_cpus} CPU{'s' if int(num_cpus) != 1 else ''})"

    fig, axes = plt.subplots(2, 2, figsize=(12, 8))
    axes = axes.flatten()

    for i, label in enumerate(labels):
        ax = axes[i]
        for scen in results.keys():
            values = [r[i] for r in results[scen]]
            ax.plot(arrival_rates, values, marker='o', label=scenario_name)
        ax.set_title(label)
        ax.set_xlabel("Avg Arrival Rate")
        ax.set_ylabel(label)
        ax.legend()
        ax.grid(True)

    plt.tight_layout()
    plt.show()