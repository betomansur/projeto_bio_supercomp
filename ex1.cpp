#include <mpi.h>
#include <omp.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <chrono>

std::map<char, int> count_bases_parallel(const std::string& sequence) {
    std::map<char, int> base_count = {{'A', 0}, {'T', 0}, {'C', 0}, {'G', 0}};
    #pragma omp parallel
    {
        std::map<char, int> local_count = {{'A', 0}, {'T', 0}, {'C', 0}, {'G', 0}};
        #pragma omp for
        for (size_t i = 0; i < sequence.size(); i++) {
            char base = sequence[i];
            if (local_count.find(base) != local_count.end()) {
                local_count[base]++;
            }
        }
        #pragma omp critical
        for (const auto& pair : local_count) {
            base_count[pair.first] += pair.second;
        }
    }
    return base_count;
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    std::ifstream input_file("dna_sequence.fasta");
    std::string line, sequence;

    auto start_time = std::chrono::high_resolution_clock::now();

    if (rank == 0) {
        if (!input_file.is_open()) {
            std::cerr << "Erro ao abrir o arquivo." << std::endl;
            sequence = "";
        } else {
            while (std::getline(input_file, line)) {
                if (line.empty() || line[0] == '>') continue;
                sequence += line;
            }
        }
    }

    int total_size = sequence.size();
    MPI_Bcast(&total_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (total_size == 0) {
        if (rank == 0) {
            std::cerr << "Arquivo vazio ou sem sequência válida." << std::endl;
        }
        MPI_Finalize();
        return 1;
    }

    sequence.resize(total_size);
    MPI_Bcast(&sequence[0], total_size, MPI_CHAR, 0, MPI_COMM_WORLD);

    int chunk_size = total_size / size;
    int start = rank * chunk_size;
    int end = (rank == size - 1) ? total_size : start + chunk_size;

    std::string local_sequence = sequence.substr(start, end - start);
    std::map<char, int> local_count = count_bases_parallel(local_sequence);

    std::map<char, int> global_count = {{'A', 0}, {'T', 0}, {'C', 0}, {'G', 0}};
    for (const auto& pair : local_count) {
        int local_value = pair.second;
        int global_value = 0;
        MPI_Reduce(&local_value, &global_value, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
        if (rank == 0) global_count[pair.first] = global_value;
    }

    if (rank == 0) {
        auto end_time = std::chrono::high_resolution_clock::now();
        double elapsed_time = std::chrono::duration<double>(end_time - start_time).count();
        
        std::cout << "Contagem de Bases:\n";
        for (const auto& pair : global_count) {
            std::cout << pair.first << ": " << pair.second << "\n";
        }
        std::cout << "Tempo total de execução: " << elapsed_time << " segundos." << std::endl;
    }

    MPI_Finalize();
    return 0;
}
