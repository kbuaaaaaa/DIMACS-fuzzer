#include "main.h"

int COUNTER = 0;
int INPUT_COUTER = 0;
int CURRENT_COUNTER = 0;
Error Errors[15];

int main(int argc, char *argv[])
{
    std::string SATPath = argv[1];
    int seed = atoi(argv[2]);
    srand(seed);

    while (true)
    {
        generate_cnf_files();
        // generate input
        for (int i = CURRENT_COUNTER; i < INPUT_COUTER; i++)
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
    // if(rand() % 2 == 0 )
    // generate_correct_cnf_files();
    // else
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

std::string generate_correct_cnf()
{
    int num_vars = 0;
    int num_clauses = 0;

    if ((rand() % (101)) < 1)
    {
        num_vars = (rand() % (50000 - 35000)) + 35000;
        num_clauses = (rand() % (5000 - 3500)) + 3500;
    }
    else
    {
        num_vars = rand() % 400 + 1;
        num_clauses = rand() % 400 + 1;
    }

    std::stringstream ss_cnf;

    ss_cnf << "p cnf " << num_vars << " " << num_clauses << "\n";
    for (int i = 0; i < num_clauses; ++i)
    {
        int num_literals = rand() % num_vars + 1;
        for (int j = 0; j < num_literals; ++j)
        {
            int literal = rand() % num_vars + 1;
            if (rand() % 2)
            {
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
    int choose_case = rand() % 60;

    if (choose_case < 20)
    {
        for (int i = 0; i < num_changes; i++)
        {
            int change = rand() % (correct.size() - 7) + 7;
            correct.at(change) = 'z';
        }
    }
    else if (choose_case < 40)
    { // add \n randombly
        for (int i = 0; i < num_changes; i++)
        {
            int change = rand() % (correct.size() - 7) + 7;

            correct.at(change) = '\n';
        }
    }
    else if (choose_case < 60)
    { // add punctuation
        for (int i = 0; i < num_changes; i++)
        {
            int change = rand() % (correct.size() - 7) + 7;
            correct.at(change) = ' ';
        }
    }
    else if (choose_case < 80)
    { // add rand punctuation
        for (int i = 0; i < num_changes; i++)
        {
            int change = rand() % (correct.size() - 7) + 7;
            correct.at(change) = VALID_PUNCTUATION[rand() % VALID_PUNCTUATION.size()];
        }
    }
    else if (choose_case < 80)
    { // injection of bytes
        for (int i = 0; i < num_changes; i++)
        {
            int change = rand() % (correct.size() - 7) + 7;
            /* code */
        }
    }
    else if (choose_case < 100)
    { // overflow clause
        for (int i = 0; i < num_changes; i++)
        {
            int change = rand() % (correct.size() - 7) + 7;
            /* code */
        }
    }

    // add rand punctuation
    // add rand special chars \0 \t \r \v \f  //
    // p cnf in random place
    // boolean operators bit wise ||1 &&0 ^
    // overflow clause
    // byte injection

    // rando all together between them

    // std::cout << correct << std::endl;
    return correct;
}

GrepReturn grep_output(const std::string &output, const std::string &pattern)
{
    std::string cmd = "echo \"" + output + "\" | grep -E \"" + pattern + "\"";
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);

    if (!pipe)
    {
        throw std::runtime_error("popen() failed!");
    }

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
    {
        result += buffer.data();
    }

    

    return GrepReturn{ result, result.empty()};
}

void save_to_file(const char *output, int i)
{
    // ASYNC WRITE HERE
    std::string name = "fuzzed-tests/test_error_" + std::to_string(i) + ".txt";
    std::string grep_content = "";
    std::ofstream error_file(name);

    for (size_t j = 0; j < 15; j++)
    {

        auto res = grep_output(output, REGEX[j]);
        grep_content += res.result + "\n";
        
        if(!res.isEmpty){
            Errors[j].count ++;
            Errors[j].filename->append("AUTOGEN_"+std::to_string(i)+".cnf");
        }

    }
    

    if (grep_content != "")
    {
        error_file << grep_content << "\n";
        error_file << output << "\n";

        std::string command = "cp inputs/AUTOGEN_" + std::to_string(i) + ".cnf fuzzed-tests/test" + std::to_string(i) + ".cnf";

        if (std::system(command.c_str()) == 0)
        {
            std::cout << "File copied successfully." << std::endl;
        }
        else
        {
            std::cerr << "File copy failed." << std::endl;
        }
    }

    error_file.close();
}

void execute(subprocess::Popen &SUTProcess)
{
    std::future<void> future = std::async(std::launch::async, [&]()
                                          { SUTProcess.wait(); });

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
