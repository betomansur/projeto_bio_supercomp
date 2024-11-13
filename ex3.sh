#!/bin/bash
#SBATCH --job-name=mpi_ex3
#SBATCH --output=mpi_ex3.out
#SBATCH --ntasks=4
#SBATCH --cpus-per-task=4
#SBATCH --mem=1G
#SBATCH --time=00:05:00
#SBATCH --partition=express

mpiexec -n 4 ./ex3
