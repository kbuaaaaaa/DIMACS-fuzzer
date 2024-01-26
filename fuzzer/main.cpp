#include "main.h"


int COUNTER = 0;
int INPUT_COUTER = 0;
int CURRENT_COUNTER = 0;

int main(int argc, char *argv[])
{
    std::string SATPath = argv[1];
    int seed = atoi(argv[2]);   
    srand(seed);

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
    if(rand() % 2 == 0 )    
        generate_correct_cnf_files();
    else
        generate_trash_cnf_files();
}

void generate_correct_cnf_files()
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

void generate_trash_cnf_files()
{
    for (int i = 0; i < 5; i++)
    {

        std::string name = "inputs/AUTOGEN_" + std::to_string(INPUT_COUTER) + ".cnf";
        std::ofstream file(name);

        file << generate_trash_cnf() << "\n";
        file.close();

        INPUT_COUTER += 1;
    }
}

std::string generate_correct_cnf() {
    int num_vars = 0;
    int num_clauses = 0;

    if((rand() % (101) ) < 1)
    {
        num_vars = (rand() % (50000-35000)) + 35000;
        num_clauses = (rand() % (5000-3500)) + 3500;
    }
    else
    {
        num_vars = rand() % 400 + 1;
        num_clauses = rand() % 400 + 1;
    }

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

std::string generate_trash_cnf()
{  
    std::string correct = generate_correct_cnf();
    int num_changes = rand() % correct.size();
    int chosecase = rand() % 60;

    if (chosecase < 20) { 
        for (int i = 0; i < num_changes; i++) {
            int change = rand() % (correct.size()-7) + 7;
            correct.at(change) = 'z';
            }
    } else if (chosecase < 40){ // add \n randombly 
        for (int i = 0; i < num_changes; i++) {
            int change = rand() % (correct.size()-7) + 7;
            correct.at(change) = '\n';
        }
    } else if (chosecase < 60) { // add punctuation
        for (int i = 0; i < num_changes; i++) {
            int change = rand() % (correct.size()-7) + 7;
            correct.at(change) = ' ';
        }
    } else if (chosecase < 80) { // injection of bytes 
        for (int i = 0; i < num_changes; i++) { 
            int change = rand() % (correct.size()-7) + 7;
            /* code */
        }
    } else if (chosecase < 100) { // overflow clause 
        for (int i = 0; i < num_changes; i++) {
            int change = rand() % (correct.size()-7) + 7;
            /* code */
        }
    }

    // add rand punctuation
    // add rand special chars \0 \n \t \r \v \f
    // p cnf in random place
    // boolean operators bit wise ||1
    // overflow clause
    // byte injection

    //rando all together between them 



    


    std::cout << correct << std::endl;
    return correct;
}

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

        save_to_file(output.second.buf.data(), COUNTER);
        COUNTER = COUNTER + 1;
    }
}

