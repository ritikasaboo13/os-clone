# OS-Clone

This repository, **OS-Clone**, contains various modules that simulate essential functionalities of an operating system. Each module is organized in a dedicated subdirectory, covering areas such as process scheduling, I/O scheduling, memory management, and linking.

## Repository Structure

The repository is organized as follows:

- **process-scheduler/**: Simulates different process scheduling algorithms.
- **io-scheduler/**: Includes implementations of various I/O scheduling algorithms.
- **linker/**: Emulates a basic linker for understanding code linking and loading.
- **mmu/**: Provides a memory management unit (MMU) simulation with page replacement strategies.

## Setup and Installation

1. **Clone the Repository**
   
   To start working with this repository, clone it to your local machine:
   ```bash
   git clone https://github.com/yourusername/os-clone.git
   cd os-clone
   ```

2. **Build Each Module**
   
   Each module has its own makefile. You can compile individual modules by navigating to their respective directories and running:
   ```bash
   cd module-directory
   make
   ```

   For example, to build the process-scheduler module, run:
   ```bash
   cd process-scheduler
   make
   ```

3. **Run the Programs**
   
   After building, you can run each module's program(s) using the provided scripts or executables. Each directory typically contains a `runit.sh` script for easy execution.

## Modules

### Process Scheduler (`process-scheduler/`)

Simulates various process scheduling algorithms:
- Round Robin
- First-Come-First-Serve (FCFS)
- Priority Scheduling

To Run:
- Use the `scheduler.cpp` file or `runit.sh` script to execute the scheduling simulations.

### I/O Scheduler (`io-scheduler/`)

Includes several I/O scheduling algorithms:
- First-Come-First-Serve (FCFS)
- Shortest Seek Time First (SSTF)
- SCAN and C-SCAN

To Run:
- Run the `io.cpp` file or use the `runit.sh` script.

### Linker (`linker/`)

Emulates linking and loading functionalities, useful for understanding basic linker operations.

To Run:
- Compile with `make` and then execute the resulting binary.

### Memory Management Unit (`mmu/`)

This module simulates a memory management unit, covering:
- Page Replacement Algorithms (e.g., FIFO, LRU)
- Memory Allocation Techniques

To Run:
- Use the makefile to compile the program and test with provided input files.

## Usage Example

Here's an example of how to compile and run the process-scheduler module:

```bash
cd process-scheduler
make
./scheduler    # or use ./runit.sh if provided
```
