#include <iostream>
#include <chrono>
#include <thread>
#include "subprocess.hpp"


int main(int argc, char *argv[])
{
    std::string SATPath = argv[1];
    std::string InputPath = argv[2];


    auto SUTProcess = subprocess::Popen({SATPath, InputPath}, subprocess::output(subprocess::PIPE), subprocess::error(subprocess::PIPE) );

    auto [output,error_output] = SUTProcess.communicate();
    
    std::cerr << "Error: " << error_output.buf.data();
    
}