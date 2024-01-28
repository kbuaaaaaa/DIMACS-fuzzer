#include "main.h"

long OUTPUT_COUNTER = 0;
long INPUT_COUNTER = 0;
long CURRENT_COUNTER = 0;
std::mutex OutputCounterMutex;
std::mutex InputCounterMutex;
std::mutex CurrentCounterMutex;

Error Errors[REGEX_ERRORS];
int FilesCopied = 0;

int main(int argc, char *argv[])
{
    // try
    // {
    //     // you can pass http::InternetProtocol::V6 to Request to make an IPv6 request
    //     http::Request request{"http://172.167.164.98/"};

    //     // send a get request
    //     const auto response = request.send("GET");
    //     std::cout << std::string{response.body.begin(), response.body.end()} << '\n'; // print the result
    // }
    // catch (const std::exception &e)
    // {
    //     std::cerr << "Request failed, error: " << e.what() << '\n';
    // }

    std::string SATPath = argv[1];
    int seed = atoi(argv[2]);
    srand(seed);

    set_edge_cases();
    run_one_time_edge_cases(SATPath);

    // generate input
    std::thread InputGenerationThread(generate_cnf_files);
    std::thread FuzzingThread1(fuzz, SATPath);
    std::thread FuzzingThread2(fuzz, SATPath);

    FuzzingThread1.join();
    FuzzingThread2.join();
    InputGenerationThread.join();
}

void fuzz(std::string SATPath){
    while(true){
        InputCounterMutex.lock();
        long MaxInput = INPUT_COUNTER;
        InputCounterMutex.unlock();
        CurrentCounterMutex.lock();
        long CurrentInput = CURRENT_COUNTER;
        CurrentCounterMutex.unlock();
        while (CurrentInput < MaxInput){
            CurrentCounterMutex.lock();
            CurrentInput = CURRENT_COUNTER++;
            CurrentCounterMutex.unlock();
            std::string InputPath = "inputs/AUTOGEN_" + std::to_string(CurrentInput) + ".cnf";
            auto SUTProcess = subprocess::Popen({SATPath, InputPath}, subprocess::output(subprocess::PIPE), subprocess::error(subprocess::PIPE));
            execute(SUTProcess);
            InputCounterMutex.lock();
            MaxInput = INPUT_COUNTER;
            InputCounterMutex.unlock();
        }
    }
}

void generate_cnf_files()
{
    while(true){
        if (rand() % 101 < 30)
            generate_correct_cnf_files();
        else
            generate_trash_cnf_files();
    }
}

void generate_correct_cnf_files()
{
    std::string name = "inputs/AUTOGEN_" + std::to_string(INPUT_COUNTER) + ".cnf";
    std::ofstream file(name);

    file << generate_correct_cnf() << "\n";
    file.close();
    InputCounterMutex.lock();
    INPUT_COUNTER++;
    InputCounterMutex.unlock();
}

void generate_trash_cnf_files()
{
    std::string name = "inputs/AUTOGEN_" + std::to_string(INPUT_COUNTER) + ".cnf";
    std::ofstream file(name);

    file << generate_trash_cnf() << "\n";
    file.close();
    InputCounterMutex.lock();
    INPUT_COUNTER++;
    InputCounterMutex.unlock();
}

std::string generate_correct_cnf()
{
    int num_vars = 0;
    int num_clauses = 0;
    int randomChance = (rand() % (101));
    std::stringstream ss_cnf;

    if (randomChance < 0)
    {
        num_vars = (rand() % (1000 - 500)) + 500;
        num_clauses = (rand() % (1000 - 500)) + 500;
    }
    else if(randomChance<50)
    {
        num_vars = rand() % 400 + 1;
        num_clauses = rand() % 400 + 1;
    }
    else
    {
        num_vars = rand() % 40 + 1;
        num_clauses = rand() % 40 + 1;
    }
    

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
    // int choose_case = rand() % 10 + 1;
    int choose_case = CURRENT_COUNTER % 10 + 1;
    bool changing_header = (rand() % 100) < 10;
    switch (choose_case)
    {
    case 1:
    {
        // adding random char
        for (int i = 0; i < num_changes; i++)
        {
            int change = 0;
            if (!changing_header)
                change = rand() % (correct.size() - 5) + 5;
            else
                change = rand() % (correct.size() - 1) + 1;
            correct.at(change) = ALPHANUMERIC[rand() % ALPHANUMERIC.size()];
        }
        break;
    }
    case 2:
    {
        // adding random new line
        for (int i = 0; i < num_changes; i++)
        {
            int change = 0;
            if (!changing_header)
                change = rand() % (correct.size() - 5) + 5;
            else
                change = rand() % (correct.size() - 1) + 1;
            correct.at(change) = '\n';
        }
        break;
    }
    case 3:
    {
        // adding random space
        for (int i = 0; i < num_changes; i++)
        {
            int change = 0;
            if (!changing_header)
                change = rand() % (correct.size() - 5) + 5;
            else
                change = rand() % (correct.size() - 1) + 1;
            correct.at(change) = ' ';
        }
        break;
    }
    case 4:
    {
        // adding random puntuation
        for (int i = 0; i < num_changes; i++)
        {
            int change = 0;
            if (!changing_header)
                change = rand() % (correct.size() - 5) + 5;
            else
                change = rand() % (correct.size() - 1) + 1;
            correct.at(change) = VALID_PUNCTUATION[rand() % VALID_PUNCTUATION.size()];
        }
        break;
    }
    case 5:
    {
        // adding random special char
        for (int i = 0; i < num_changes; i++)
        {
            int change = 0;
            if (!changing_header)
                change = rand() % (correct.size() - 5) + 5;
            else
                change = rand() % (correct.size() - 1) + 1;
            correct.at(change) = ESCAPE_CHARS[rand() % ESCAPE_CHARS.size()];
        }
        break;
    }
    case 6:
    {
        // adding "p cnf" in random places
        for (int i = 0; i < num_changes; i++)
        {
            int change = 0;
            if (!changing_header)
                change = rand() % (correct.size() - 5) + 5;
            else
                change = rand() % (correct.size() - 1) + 1;
            correct.insert(change, "p cnf ");
        }
        break;
    }
    case 7:
    {
        // adding boolean operators bit wise ||1 &&0 ^
        for (int i = 0; i < num_changes; i++)
        {
            int change = 0;
            if (!changing_header)
                change = rand() % (correct.size() - 5) + 5;
            else
                change = rand() % (correct.size() - 1) + 1;
            correct.insert(change, BITWISE_OPERATOR[rand() % BITWISE_OPERATOR.size()]);
        }
        break;
    }
    case 8:
    {
        // adding long long max
        for (int i = 0; i < num_changes; i++)
        {
            int change = 0;
            if (!changing_header)
                change = rand() % (correct.size() - 5) + 5;
            else
                change = rand() % (correct.size() - 1) + 1;
            correct.insert(change, OVERFLOW_LITERAL);
        }
        break;
    }
    case 9:
    {
        // adding random bytes
        for (int i = 0; i < num_changes; i++)
        {
            int change = 0;
            if (!changing_header)
                change = rand() % (correct.size() - 5) + 5;
            else
                change = rand() % (correct.size() - 1) + 1;
            correct.insert(change, RANDOM_BYTES[rand() % RANDOM_BYTES.size()]);
        }
        break;
    }
    case 10:
    {
        // adding many different random things
        for (int i = 0; i < num_changes; i++)
        {
            int change = 0;
            if (!changing_header)
                change = rand() % (correct.size() - 5) + 5;
            else
                change = rand() % (correct.size() - 1) + 1;
            correct.insert(change, RANDOM_ALL[rand() % RANDOM_ALL.size()]);
        }
        break;
    }
    default:
        break;
    }
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

    return GrepReturn{result, result.empty()};
}

void save_to_file(const char *output, long i)
{
    // ASYNC WRITE HERE
    std::string name = "fuzzed-tests/test_error_" + std::to_string(i) + ".txt";
    std::string grep_content = "";
    std::ofstream error_file(name);

    for (size_t j = 0; j < REGEX_ERRORS; j++)
    {

        auto res = grep_output(output, REGEX[j]);

        if (!res.isEmpty)
        {
            if (FilesCopied >= 20)
            {
                Error *maxError = &(Errors[0]);
                for (int k = 1; k < REGEX_ERRORS; k++)
                {
                    if (Errors[k].count > maxError->count)
                    {
                        maxError = &(Errors[k]);
                    }
                }

                std::string command = "rm fuzzed-tests/" + maxError->filename[maxError->count - 1];

                if (std::system(command.c_str()) == 0)
                {
                    maxError->filename[maxError->count - 1].clear();
                    maxError->count--;
                    FilesCopied--;
                    std::cout << "File removed successfully. " << command.c_str() << std::endl;
                }
                else
                {
                    std::cerr << "File remove failed. " << command.c_str() << std::endl;
                }
            }

            grep_content += res.result + "\n";
            Errors[j].filename[Errors[j].count] = "AUTOGEN_" + std::to_string(i) + ".cnf";
            Errors[j].count++;
        }
    }

    if (grep_content != "")
    {
        error_file << grep_content << "\n";
        error_file << output << "\n";

        std::string command = "cp inputs/AUTOGEN_" + std::to_string(i) + ".cnf fuzzed-tests/AUTOGEN_" + std::to_string(i) + ".cnf";

        if (std::system(command.c_str()) == 0)
        {
            FilesCopied++;
            std::cout << "File copied successfully. " << command.c_str() << std::endl;
        }
        else
        {
            std::cerr << "File copy failed. " << command.c_str() << std::endl;
        }
    }

    printf("----------------------------------------------------\n");

    for(int llll = 0; llll< REGEX_ERRORS;llll++){
        printf("Error: %d   Appeared: %d times\n",llll,Errors[llll].count);
    }
    printf("----------------------------------------------------\n");

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
        OutputCounterMutex.lock();
        long CurrentOutput = OUTPUT_COUNTER++;
        OutputCounterMutex.unlock();
        save_to_file("SAT killed timeout reached -> ERROR: Infinite LOOP\n", CurrentOutput);
    }
    else if (SUTProcess.retcode() != 0)
    {
        auto output = SUTProcess.communicate();
        OutputCounterMutex.lock();
        long CurrentOutput = OUTPUT_COUNTER++;
        OutputCounterMutex.unlock();
        save_to_file(output.second.buf.data(), CurrentOutput);
    }
}

void set_edge_cases()
{
    std::string name = "inputs/AUTOGEN_" + std::to_string(INPUT_COUNTER) + ".cnf";
    std::ofstream file(name);

    file << "";
    file.close();

    INPUT_COUNTER += 1;

    name = "inputs/AUTOGEN_" + std::to_string(INPUT_COUNTER) + ".cnf";
    file.open(name);
    file << "p cnf "
         << "1000000000000"
         << " "
         << "1000000000000"
         << "\n";
    file << "1000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000 0\n";
    file.close();

    INPUT_COUNTER += 1;

    name = "inputs/AUTOGEN_" + std::to_string(INPUT_COUNTER) + ".cnf";
    file.open(name);

    file << "p cnf 1 1\n";
    file << "-1 0\n";
    file.close();

    INPUT_COUNTER += 1;

    name = "inputs/AUTOGEN_" + std::to_string(INPUT_COUNTER) + ".cnf";
    file.open(name);

    file << "p cnf 1 1\n";
    file << "1 0\n";
    file.close();

    INPUT_COUNTER += 1;

    name = "inputs/AUTOGEN_" + std::to_string(INPUT_COUNTER) + ".cnf";
    file.open(name);

    file << "p cnf 1 1\n";
    file << "0 0\n";
    file.close();

    INPUT_COUNTER += 1;

    name = "inputs/AUTOGEN_" + std::to_string(INPUT_COUNTER) + ".cnf";
    file.open(name);

    file << "p cnf 1 1\n";
    file << "1 1 1 0\n";
    file.close();

    INPUT_COUNTER += 1;
}

void run_one_time_edge_cases(std::string SATPath)
{
    for (int i = CURRENT_COUNTER; i < INPUT_COUNTER; i++)
    {
        std::string InputPath = "inputs/AUTOGEN_" + std::to_string(i) + ".cnf";
        auto SUTProcess = subprocess::Popen({SATPath, InputPath}, subprocess::output(subprocess::PIPE), subprocess::error(subprocess::PIPE));
        execute(SUTProcess);
    }
    CURRENT_COUNTER = INPUT_COUNTER;

    // std::string InputPath = "inputs/WRONG_NAME.cnf";
    // auto SUTProcess = subprocess::Popen({SATPath, InputPath}, subprocess::output(subprocess::PIPE), subprocess::error(subprocess::PIPE));
    // execute(SUTProcess);
    // INPUT_COUNTER += 1;
    // CURRENT_COUNTER = INPUT_COUNTER;
}