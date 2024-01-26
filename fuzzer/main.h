#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <future>
#include "subprocess.hpp"
#include <sstream>


#pragma once
void execute(subprocess::Popen &SUTProcess);
//std::string random_gen_input(int seed);
void generate_cnf_files();
std::string generate_correct_cnf();