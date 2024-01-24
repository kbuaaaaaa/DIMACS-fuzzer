#include <iostream>
#include <chrono>
#include <thread>
#include <future>
#include "subprocess.hpp"

int main(int argc, char *argv[])
{
    std::string SATPath = argv[1];
    std::string InputPath = argv[2];

    auto SUTProcess = subprocess::Popen({SATPath, InputPath}, subprocess::output(subprocess::PIPE), subprocess::error(subprocess::PIPE));

    std::future<std::pair<subprocess::Buffer,subprocess::Buffer>> future = std::async(std::launch::async, [&SUTProcess]{
        return SUTProcess.communicate();
    });

    if (future.wait_for(std::chrono::seconds(5)) == std::future_status::timeout)
    {
        std::cerr << "SAT killed timeout reached -> ERROR: Infinite LOOP\n";
        SUTProcess.kill(15);
    }
    else
    {
        auto [output, error_output] = future.get();
        std::cout << output.buf.data();
        std::cerr << "Error: " << error_output.buf.data();
    }
}