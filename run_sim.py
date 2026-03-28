import subprocess
import sys
import matplotlib.pyplot as plt

def run_cpp_program(executable_path, args=None):
    command = [executable_path] + (args or [])
    result = subprocess.run(command, capture_output=True, text=True)
    
    # output_lines = result.stdout.splitlines()
    
    # if result.stderr:
    #     print(result.stderr, end="", file=sys.stderr)
    
    # return output_lines, result.returncode
    return result.returncode

if __name__ == "__main__":
    if len(sys.argv) < 4:
        print("Usage: run_sim.py <executable> <avg_arrival_rate> <avg_service_rate>")
        sys.exit(1)
    
    # lines, exit_code = run_cpp_program(sys.argv[1], sys.argv[2:])
    exit_code = run_cpp_program(sys.argv[1], sys.argv[2:])
    
    # Convert to numbers
    # results = [float(x) for x in lines]

    # # Print each value
    # for i, val in enumerate(results):
    #     print(f"Line {i+1}: {val}")

    # # Plot
    # plt.plot(results, marker='o')
    # plt.title("C++ Program Output")
    # plt.xlabel("Line")
    # plt.ylabel("Value")
    # plt.grid(True)
    # plt.tight_layout()
    # plt.show()

    sys.exit(exit_code)