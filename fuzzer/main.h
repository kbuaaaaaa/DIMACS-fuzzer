#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <future>
#include "subprocess.hpp"
#include <sstream>
#include <climits>
#include <vector>

std::string NORM_HEADER = "p cnf 10 10\n";
std::string OVERFLOW_HEADER = "p cnf " + std::to_string(LLONG_MAX + 1) + " " + std::to_string(LLONG_MAX + 1) + "\n";
std::string RANDOM_HEADER = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~ \t\n\r\x0B\x0C";
std::vector<std::string> SPECIAL_INPUT = {
    "",
    "p cnf\n",
    OVERFLOW_HEADER,
    RANDOM_HEADER,
    NORM_HEADER,
    NORM_HEADER + "!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~" + "\n",
    NORM_HEADER + RANDOM_HEADER + "\n"
};

#pragma once
void execute(subprocess::Popen &SUTProcess);
//std::string random_gen_input(int seed);
void generate_cnf_files();
std::string generate_correct_cnf();
std::string generate_trash_cnf();

