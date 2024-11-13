#include <mpi.h>
#include <omp.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <chrono>

// Tabela de tradução RNA -> Aminoácidos representados por números
std::unordered_map<std::string, int> codon_table = {
    {"CCA", 1}, {"CCG", 1}, {"CCU", 1}, {"CCC", 1}, // Prolina
    {"UCU", 2}, {"UCA", 2}, {"UCG", 2}, {"UCC", 2}, // Serina
    {"CAG", 3}, {"CAA", 3},                         // Glutamina
    {"ACA", 4}, {"ACC", 4}, {"ACU", 4}, {"ACG", 4}, // Treonina
    {"AUG", 5},                                     // Metionina (início)
    {"UGA", 0}, {"UAA", 0}, {"UAG", 0},             // STOP
    {"UGC", 6}, {"UGU", 6},                         // Cisteína
    {"GUG", 7}, {"GUA", 7}, {"GUC", 7}, {"GUU", 7}  // Valina
};

// Função para traduzir uma sequência de RNA em aminoácidos até um códon STOP
std::vector<int> translate_rna_to_protein(const std::string& rna_sequence) {
    std::vector<int> protein;
    bool stop_found = false;

    #pragma omp parallel
    {
        std::vector<int> local_protein;
        #pragma omp for
        for (size_t i = 0; i <= rna_sequence.size() - 3; i += 3) {
            if (stop_found) continue;

            std::string codon = rna_sequence.substr(i, 3);
            if (codon_table.find(codon) != codon_table.end()) {
                int amino_acid = codon_table[codon];
                if (amino_acid == 0) {
                    stop_found = true; // Marca que STOP foi encontrado
                } else {
                    local_protein.push_back(amino_acid);
                }
            }
        }

        #pragma omp critical
        protein.insert(protein.end(), local_protein.begin(), local_protein.end());
    }
    return protein;
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
    std::vector<int> local_protein = translate_rna_to_protein(local_sequence);

    int local_size = local_protein.size();
    std::vector<int> global_sizes(size);
    MPI_Gather(&local_size, 1, MPI_INT, global_sizes.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);

    std::vector<int> global_displs(size);
    if (rank == 0) {
        global_displs[0] = 0;
        for (int i = 1; i < size; i++) {
            global_displs[i] = global_displs[i - 1] + global_sizes[i - 1];
        }
    }

    std::vector<int> global_protein;
    if (rank == 0) {
        global_protein.resize(global_displs[size - 1] + global_sizes[size - 1]);
    }

    MPI_Gatherv(local_protein.data(), local_size, MPI_INT, global_protein.data(),
                global_sizes.data(), global_displs.data(), MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        auto end_time = std::chrono::high_resolution_clock::now();
        double elapsed_time = std::chrono::duration<double>(end_time - start_time).count();

        std::cout << "Tradução concluída.\n";
        std::cout << "Sequência traduzida em aminoácidos (números):\n";
        for (int amino_acid : global_protein) {
            if (amino_acid == 0) break; // Parar ao encontrar STOP
            std::cout << amino_acid << " ";
        }
        std::cout << "\nTempo total de execução: " << elapsed_time << " segundos." << std::endl;
    }

    MPI_Finalize();
    return 0;
}
