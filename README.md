# Parallel Programming with Pthreads - Rainfall Simulation Project

## Overview

This project, part of the ECE 565 Performance Optmization & Parallelism - Fall 2023 course, focuses on parallel programming using Pthreads. It includes the development of a rainfall simulation program in a 2D landscape, both in sequential and parallel versions. The simulation considers various factors like raindrop absorption, trickling to lower elevations, and time step dynamics.

## Project Description

### Rainfall Simulation
- **Objective:** Simulate raindrops falling onto a 2D landscape grid, their absorption into the ground, and trickling to neighboring points with lower elevations.
- **Dimensions:** NxN grid with each point having an integer elevation.

### Simulation Dynamics
1. **Raindrops Falling:** Raindrops fall onto each point of the landscape during the first M timesteps.
2. **Absorption:** Raindrops at each point are partially absorbed into the ground at a specified rate.
3. **Trickling:** Remaining raindrops trickle to the neighboring point(s) with the lowest elevation.

### Program Characteristics
- **Sequential Version:** Developed without parallelization.
- **Parallel Version:** Utilizes pthreads for parallel execution.
- **Command Line Arguments:** The program takes five arguments including number of threads, simulation time steps, absorption rate, landscape dimension, and elevation file.

## Development Environment

- **Primary Languages:** C or C++.
- **Parallelization Tool:** Pthreads or C++ threads.
- **Execution Platforms:** Personal machine, Intel server (kraken.egr.duke.edu or leviathan.egr.duke.edu), or an 8-core Ubuntu VM.
- **Performance Experiments:** Conducted on an 8-core VM running Ubuntu Linux.

## Key Challenges

- **Parallel Code Debugging:** Ensuring the code is free from race conditions and other parallel programming bugs.
- **Design Effort:** Significant effort required in design, despite a relatively small amount of code.
- **Timing Conditions:** Handling bugs in parallel code which are often timing dependent.

## Installation and Execution

### Prerequisites
- Access to an 8-core VM or equivalent system.
- C or C++ development environment.
- Pthread library for parallel programming.

### Running the Program
1. Compile the sequential and parallel versions using the provided Makefile.
2. Execute using the command `./rainfall_seq` for the sequential version and `./rainfall_pt` for the parallel version with appropriate command line arguments.

## Reporting and Submission

### Report Content
- **Sequential Algorithm Description:** Detailing the approach and data structures used.
- **Parallelization Strategy:** Discussion on the aspects of code parallelized and synchronization used.
- **Performance Analysis:** Comparing performance across different thread counts and with the sequential code.

## Contributions

This project was completed as part of an academic assignment with requirments provided requirments.pdf. Contributions were made solely by Koushik Annareddy Sreenath, adhering to the project guidelines and requirements set by the course ECE-565 Performance Optmization & Parallelism. 

## License

This project is part of an academic course and is subject to university guidelines and policies regarding academic integrity and use.

## Acknowledgments

- Thanks to Brian Rogers and the course staff for providing guidance and support throughout the project.

