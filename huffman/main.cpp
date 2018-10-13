#include "huffman.h"
#include <iostream>
#include <fstream>

int main(int argc, char * argv[]) {
    if (argc != 4) {
        std::cout << "Format: <-e | -d> from to\n";
        return 0;
    }
    auto opt = std::string(argv[1]);
    auto from = std::string(argv[2]);
    auto to = std::string(argv[3]);
    std::ifstream input(from, std::ifstream::binary);
    std::ofstream output(to, std::ofstream::binary);
    if (!input.is_open() || !output.is_open()) {
        std::cout << "Can not open file\n";
        return 0;
    }
    if (opt == "-e") {
        huffman::encode(input, output);
    } else if (opt == "-d") {
        if (!huffman::decode(input, output)) {
            std::cout << "Invalid file\n";
        }
    } else {
        std::cout << "Format: <-e | -d> from to\n";
    }
    return 0;
}