#include "main.h"


int COUNTER = 0;
int INPUT_COUTER = 0;
int CURRENT_COUNTER = 0;

int main(int argc, char *argv[])
{
    std::string SATPath = argv[1];
//  std::string InputPath = argv[2];
///
    int seed = atoi(argv[2]);   
    srand(seed);
///

    while (true)
    {
        generate_cnf_files();
        // generate input
        for(int i = CURRENT_COUNTER; i < INPUT_COUTER; i++)
        {
            std::string InputPath = "inputs/AUTOGEN_" + std::to_string(i) + ".cnf";
            auto SUTProcess = subprocess::Popen({SATPath, InputPath}, subprocess::output(subprocess::PIPE), subprocess::error(subprocess::PIPE));
            execute(SUTProcess);
        }
        CURRENT_COUNTER = INPUT_COUTER;
    }
    
}

void generate_cnf_files()
{
    for (int i = 0; i < 5; i++)
    {

        std::string name = "inputs/AUTOGEN_" + std::to_string(INPUT_COUTER) + ".cnf";
        std::ofstream file(name);

        file << generate_correct_cnf() << "\n";
        file.close();

        INPUT_COUTER += 1;
    }
}

// std::string random_gen_input() 
// {
//     std::string input = "p cnf ";

//     return "";
// }

///
std::string generate_correct_cnf() {
    int num_vars = rand() % 4000 + 1;
    int num_clauses = rand() % 4000 + 1;
    std::stringstream ss_cnf;

    ss_cnf << "p cnf " << num_vars << " " << num_clauses << "\n";
    for (int i = 0; i < num_clauses; ++i) {
        int num_literals = rand() % num_vars + 1;
        for (int j = 0; j < num_literals; ++j) {
            int literal = rand() % num_vars + 1;
            if (rand() % 2) {
                literal = -literal;
            }
            ss_cnf << literal << " ";
        }
        ss_cnf << "0\n";
    }
    return ss_cnf.str();
}
/// 

void save_to_file(const char *output, int i)
{
    //ASYNC WRITE HERE
    std::string name = "fuzzed-tests/test" + std::to_string(i) + ".txt";
    std::ofstream file(name);

    file << output << "\n";
    file.close();
}

void execute(subprocess::Popen &SUTProcess)
{
    std::future<void> future = std::async(std::launch::async, [&]()
                                          { SUTProcess.wait();});

    if (future.wait_for(std::chrono::seconds(15)) == std::future_status::timeout)
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

