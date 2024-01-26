#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <future>
#include "subprocess.hpp"
#include <random>


#pragma once
void execute(subprocess::Popen &SUTProcess);
std::string random_gen_input(int seed);
