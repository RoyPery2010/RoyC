#include <iostream>
#include <fstream>
#include <optional>
#include <sstream>
#include <vector>
#include "./generation.hpp"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Incorrect usage. Correct usage  is..." << std::endl;
        std::cerr << "RoyC <input.rc>" << std::endl;
        return EXIT_FAILURE;
    }
    std::string contents;
    {
        std::stringstream contents_stream;
        std::fstream input(argv[1], std::ios::in);
        contents_stream << input.rdbuf();
        contents = contents_stream.str();
    }
    Tokenizer tokenizer(contents);
    std::vector<Token> tokens = tokenizer.tokenize();
    for ([[maybe_unused]] const auto& token : tokens) {
        std::string val = token.value.has_value() ? token.value.value() : "";
        std::cout << "Token type: " << (unsigned) token.type
                  << " Value: " << val << std::endl;
    }
    Parser parser(tokens);
    std::optional<NodeProg> prog = parser.parse_prog();
    if (!prog.has_value()) {
        std::cerr << "Invalid program" << std::endl;
        exit(EXIT_FAILURE);
    }
    {
        Generator generator(prog.value());
        std::string program;
        try {
            program = generator.gen_prog();
        }
        catch (...) {
            std::cout << "Failed to generate program" << std::endl;
            exit(EXIT_FAILURE);
        }
        std::ofstream file("out.asm");
        std::cout << program << std::endl << std::flush;
        file << program;
        file.close();
    }
    system("nasm -o out.o -fwin64 out.asm");
    system("ld -o out.exe out.o");
    return EXIT_SUCCESS;
};