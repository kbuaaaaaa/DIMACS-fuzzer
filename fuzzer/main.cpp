#include "main.h"


int COUNTER = 0;

int main(int argc, char *argv[])
{
    std::string SATPath = argv[1];
//  std::string InputPath = argv[2];

///
    int seed = atoi(argv[3]);
    std::string random_input = random_gen_input(seed);
///


    std::string InputPath = "inputs/" + INPUT_COUTER.to_string() + ".txt"; 

    // seed
    while (true)
    {
        // generate input

        auto SUTProcess = subprocess::Popen({SATPath, InputPath}, subprocess::output(subprocess::PIPE), subprocess::error(subprocess::PIPE));

        execute(SUTProcess);
        
    }
    
}

std::string random_gen_input(int seed) 
{
    return "";
}

void save_to_file(char *output, int i)
{
    //ASYNC WRITE HERE
    std::string name = "fuzzed-tests/test" + std::to_string(i) + ".txt";
    std::ofstream file(name);

    file << output << "\n";
    file.close();
}

void execute(subprocess::Popen &SUTProcess, std::string rand_input)
{
    std::future<void> future = std::async(std::launch::async, [&]()
                                          { SUTProcess.communicate(rand_input); });

    if (future.wait_for(std::chrono::seconds(5)) == std::future_status::timeout)
    {
        std::cerr << "SAT killed timeout reached -> ERROR: Infinite LOOP\n";
        SUTProcess.kill(15);

        save_to_file("SAT killed timeout reached -> ERROR: Infinite LOOP\n", COUNTER);
        COUNTER = COUNTER + 1;
    }
    else if (SUTProcess.retcode() != 0)
    {
        auto output = SUTProcess.communicate();
        std::cout << output.first.buf.data();

        save_to_file(output.second.buf.data(), COUNTER);
        COUNTER = COUNTER + 1;
    }
}

