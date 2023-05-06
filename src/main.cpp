//
//  main.cpp
//  scat
//
//  Created by Alex Tarasov on 2/5/20.
//  Copyright © 2020 Alex Tarasov. All rights reserved.
//

#include "basicElfWrapper.hpp"

#include "Arm/Arm.hpp"
#include "Bitstream.hpp"
#include "ParseTree.hpp"
#include "Tokenizer.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

std::stringstream joinFiles(std::vector<std::string> paths) {
    std::stringstream jointFile;
    for (auto& path : paths) {
        if(!std::filesystem::exists(path)) throw std::runtime_error("input "+path+" does not exist.");
        if(std::filesystem::is_directory(path)) throw std::runtime_error("input "+path+" is a directory");
        ifstream file{path};
        jointFile << file.rdbuf() << std::endl;
        file.close();
        jointFile.clear();
    }
    jointFile.seekg(0, std::ios::beg);
    jointFile.clear();
    return jointFile;
}

//"Examples/square_rpi3_v.s"
void compileArm(
    std::vector<std::string> input, std::string output, std::string mode = "elf", ostream* dumpLog = nullptr
) {
    Tokenizer tkn;
    ParseTreeBuilder ptb;
    Arm64 arm64;
    Logging log;

    tkn.log.forward(std::cerr);
    ptb.log.forward(std::cerr);
    arm64.log.forward(std::cerr);
    if (dumpLog) { log.forward(*dumpLog); }

    log << "------------------------" << endl;

    std::stringstream infile = joinFiles(input);

    auto tokens = tkn.tokenize(infile);
    log << "------------" << endl;

    tokens.insert(tokens.begin(), Token("[", "START"));
    tokens.push_back(Token("]", "END"));

    for (Token token : tokens) log << token.toString() << endl;

    log << "------------------------" << endl;

    auto tree = ptb.buildParseTree(tokens);
    log << "------------" << endl;
    log << tree.toString() << endl;

    log << "------------------------" << endl;

    auto normalTree = ptb.normalize(tree);
    log << "------------" << endl;
    log << normalTree.toString() << endl;

    log << "------------------------" << endl;

    auto machineCode = arm64.assemble(normalTree);
    log << "------------" << endl;
    //log << machineCode.getHexDWords() << endl;

    if (mode == "elf") {
        wrapElf(machineCode.getBits(), arm64.getOffset(), output);
    } else {
        ofstream binFile(output, ios::out | ios::binary);
        for (auto b : machineCode.getBits()) binFile << b;
        binFile.close();
    }
}

int main(int argc, const char* argv[]) {
    try {
        if (argc < 2) throw std::runtime_error("Not enough arguments, first arg should be either elf or raw.");
        std::string mode = std::string(argv[1]);
        if (mode != "elf" && mode != "raw")
            throw std::runtime_error("Bad argument, first arg should be either elf or raw.");
        if (argc < 4)
            throw std::runtime_error(
                "Not enough arguments, starting from the second argument, provide input paths, the last path is the putput path."
            );
        std::vector<std::string> input = {};
        for(int i = 2; i < argc-1; i++){
            input.push_back(argv[i]);
        }
        std::filesystem::path output = std::string(argv[argc-1]);
        try {
            std::filesystem::create_directories(output.parent_path());
        } catch (...) {}
        compileArm(input, output, mode);
        std::filesystem::permissions(
            output,
            std::filesystem::perms::owner_all | std::filesystem::perms::group_all,
            std::filesystem::perm_options::add
        );
    } catch (std::runtime_error& e) { std::cerr << e.what() << "\n"; } catch (...) {
        std::cerr << "Failed for unknown reason.\n";
    }
    return 0;
}
