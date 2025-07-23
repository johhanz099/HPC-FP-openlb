# OpenLB Parallelization Study in Laminar Fluids Examples

Welcome to the **OpenLB Parallelization Study** repository. In this project, we explore and benchmark OpenLB examples by modifying geometries in two laminar examples, integrating MPI, and measuring parallel performance (speedup and efficiency).

## Repository Structure
```text

├── olb-release/ # OpenLB source and examples
  ├── config.mk # MPI-enabled configuration
  ├── Makefile  # Make first to build the embedded dependencies
  ├── src/ # source files
  └── simul-square2d/ # 2D square-obstacle example
    ├── Makefile
    └── [output dirs] # tmp/, data/, fig/
  ├── examples/
   ├── laminar/
    ├── cylinder3d/ # 3D wing-profile example
      ├── Makefile
      └── [output dirs] # tmp/, data/, fig/
``` 

## Features

- **Modified Geometries**  
  - 2D example: circular obstacle → square  
  - 3D example: cylindrical obstacle → airfoil profile
- **MPI Parallelization**  
  - Configured via `config.mk`  
  - Benchmarked using speedup and parallel efficiency curves
- **Automated Metrics**  
  - `metrics` target runs multiple thread-count experiments  
  - Generates raw logs and plots (via Gnuplot)


## Docker file

To build and run with Docker

- docker build -t name-image .
- docker run -it --rm -v $(pwd):/workspace name-image

## Makefile targets

- `make`: Compiles the main program.
- `make run_simul`: Runs the executable by asking the user how many threads to use.
- `make metrics`: Runs the run.sh script to obtain the metrics data and generate the graphs with metrics.gp.
- `make clean_simul`: Removes all created directories to clean up the directory.
- `make clean`: Removes all compiled files to clean up the directory.


## Usage

To use this repository, you can start by cloning it. The provided Dockerfile ensures that all requirements for running the programs are satisfied. You can then run the run-dependencies.sh script to build the necessary dependencies.

Afterward, you can enter one of the example directories:

- cd olb-release/simul-square2d

- cd olb-release/examples/laminar/cylinder3d

Inside these directories, make sure to run make initially, and then you can execute the desired Makefile target.

Note: When running an example, OpenLB automatically generates simulation files inside the tmp folder, where you will find files for visualizing the simulation. The .vtk files can be opened with ParaView for visualization.



