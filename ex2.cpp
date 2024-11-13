#include <mpi.h>
#include <omp.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>

std::string transcribe_dna_to_rna(const std::string& dna_sequence) {
    std::string rna_sequence = dna_sequence;
    #pragma omp parallel for
    for (size_t i = 0; i < dna_sequence.size(); i++) {
        if (dna_sequence[i] == 'T') {
            rna_sequence[i] = 'U';
        } else {
            rna_sequence[i] = dna_sequence[i];
        }
    }
    return rna_sequence;
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
    std::string local_rna_sequence = transcribe_dna_to_rna(local_sequence);

    std::vector<char> global_rna_sequence(total_size);
    MPI_Gather(local_rna_sequence.data(), local_rna_sequence.size(), MPI_CHAR,
               global_rna_sequence.data(), local_rna_sequence.size(), MPI_CHAR, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        auto end_time = std::chrono::high_resolution_clock::now();
        double elapsed_time = std::chrono::duration<double>(end_time - start_time).count();

        std::cout << "Transcrição concluída.\n";
        std::cout << "Tempo total de execução: " << elapsed_time << " segundos." << std::endl;

        std::ofstream output_file("rna_sequence.fasta");
        if (output_file.is_open()) {
            output_file << "> RNA Sequence\n";
            output_file.write(global_rna_sequence.data(), total_size);
            output_file.close();
            std::cout << "Sequência RNA salva em 'rna_sequence.fasta'.\n";
        } else {
            std::cerr << "Erro ao salvar o arquivo RNA.\n";
        }
    }

    MPI_Finalize();
    return 0;
}
