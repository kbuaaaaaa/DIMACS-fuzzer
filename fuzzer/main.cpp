
#include "main.h"

int main(int argc, char *argv[])
{
    std::string SATPath = argv[1];
    std::string InputPath = argv[2];

    auto SUTProcess = subprocess::Popen({SATPath, InputPath}, subprocess::output(subprocess::PIPE), subprocess::error(subprocess::PIPE));

    execute(SUTProcess);
}

void save_to_file(char *output, int i)
{
    std::string name = "fuzzed-tests/test" + std::to_string(i) + ".txt";
    std::ofstream file(name);

    file << output << "\n";
    file.close();
}

void execute(subprocess::Popen &SUTProcess)
{
    std::future<void> future = std::async(std::launch::async, [&]()
                                          { SUTProcess.wait(); });

    if (future.wait_for(std::chrono::seconds(5)) == std::future_status::timeout)
    {
        std::cerr << "SAT killed timeout reached -> ERROR: Infinite LOOP\n";
        SUTProcess.kill(15);
    }
    else if (SUTProcess.retcode() != 0)
    {
        auto output = SUTProcess.communicate();
        std::cout << output.first.buf.data();
        
        save_to_file(output.second.buf.data(), 1);
        std::cerr << "Error: " << output.second.buf.data();
    }
}

