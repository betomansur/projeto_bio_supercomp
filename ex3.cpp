#include <mpi.h>
#include <omp.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>

// Função para contar códons de início (AUG) em uma sequência de RNA
int count_start_codons(const std::string& rna_sequence) {
    int count = 0;
    #pragma omp parallel for reduction(+:count)
    for (size_t i = 0; i <= rna_sequence.size() - 3; i += 3) {
        if (rna_sequence.substr(i, 3) == "AUG") {
            count++;
        }
    }
    return count;
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    std::ifstream input_file("rna_sequence.fasta");
    std::string line, rna_sequence;

    auto start_time = std::chrono::high_resolution_clock::now();

    if (rank == 0) {
        if (!input_file.is_open()) {
            std::cerr << "Erro ao abrir o arquivo RNA." << std::endl;
            rna_sequence = "";
        } else {
            while (std::getline(input_file, line)) {
                if (line.empty() || line[0] == '>') continue;
                rna_sequence += line;
            }
        }
    }

    int total_size = rna_sequence.size();
    MPI_Bcast(&total_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (total_size == 0) {
        if (rank == 0) {
            std::cerr << "Arquivo RNA vazio ou sem sequência válida." << std::endl;
        }
        MPI_Finalize();
        return 1;
    }

    rna_sequence.resize(total_size);
    MPI_Bcast(&rna_sequence[0], total_size, MPI_CHAR, 0, MPI_COMM_WORLD);

    int chunk_size = (total_size / 3) / size * 3; // Garantir divisão alinhada aos códons
    int start = rank * chunk_size;
    int end = (rank == size - 1) ? total_size : start + chunk_size;

    std::string local_sequence = rna_sequence.substr(start, end - start);
    int local_count = count_start_codons(local_sequence);

    int global_count = 0;
    MPI_Reduce(&local_count, &global_count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        auto end_time = std::chrono::high_resolution_clock::now();
        double elapsed_time = std::chrono::duration<double>(end_time - start_time).count();

        std::cout << "Número total de proteínas inicializadas (códons AUG): " << global_count << "\n";
        std::cout << "Tempo total de execução: " << elapsed_time << " segundos." << std::endl;
    }

    MPI_Finalize();
    return 0;
}
