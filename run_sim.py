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
    if len(sys.argv) < 3:
        print("Usage: run_sim.py <executable> <avg_service_time>")
        sys.exit(1)

    executable = sys.argv[1]
    service_time = sys.argv[2]
    arrival_rates = list(range(10, 31))

    # results[policy] = list of [turnaround, throughput, utilization, avg_waiting] per arrival rate
    results = {0: [], 1: []}

    for arrival_rate in arrival_rates:
        for policy in [0, 1]:
            lines, exit_code = run_cpp_program(executable, [str(arrival_rate), service_time, str(policy)])
            if exit_code != 0:
                print(f"Error: arrival_rate={arrival_rate}, policy={policy}", file=sys.stderr)
                results[policy].append([None] * 4)
                continue
            results[policy].append([float(x) for x in lines])

    labels = ["Avg Turnaround Time", "Total Throughput", "CPU Utilization", "Avg Processes Waiting"]
    policy_names = {0: "FCFS", 1: "SJF"}

    fig, axes = plt.subplots(2, 2, figsize=(12, 8))
    axes = axes.flatten()

    for i, label in enumerate(labels):
        ax = axes[i]
        for policy in [0, 1]:
            values = [r[i] for r in results[policy]]
            ax.plot(arrival_rates, values, marker='o', label=policy_names[policy])
        ax.set_title(label)
        ax.set_xlabel("Avg Arrival Rate")
        ax.set_ylabel(label)
        ax.legend()
        ax.grid(True)

    plt.tight_layout()
    plt.show()