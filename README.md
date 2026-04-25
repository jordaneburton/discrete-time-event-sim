# discrete-time-event-sim
A discrete time event simulator for an M/M/1 queuing system representing CPU scheduling, based on First-Come First-Served (FCFS) and Shortest Job First (SJF) algorithms. The project runs a Python program to visualize the data, while C++ is used to do calculations.

## Requirements
The following must be installed on your system in order to run:
- Python - I used Python3 but any version should work
- Matplotlib - a Python library used for graphing and plotting data
- C++ Compiler - I used g++ compiler, but you just need to be able to create an executable

## How to Run
Once you have fulfilled the requirements, simply compile your C++ program and run the python script:
```
g++ <cpp_file> <cpp_exe>

python3 run_sim.py ./<cpp_exe> <average service time>
```
Note that if the average service time is too large, the scheduler becomes overloaded and the simulator might stall or error out.