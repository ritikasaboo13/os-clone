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

## Build Each Module

Each module has its own makefile. You can compile individual modules by navigating to their respective directories and running:

   ```bash
   cd module-directory
   make
