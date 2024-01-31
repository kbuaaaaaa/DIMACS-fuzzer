#include "main.h"

long INPUT_COUNTER = 0;
long CURRENT_COUNTER = 0;
Error Errors[REGEX_ERRORS];
int FilesCopied = 0;

std::mutex InputCounterMutex;
std::mutex CurrentCounterMutex;

ThreadSafeQueue<SaveToFileParams> saveToFileQueue;

int main(int argc, char *argv[])
{
    std::string SATPath = argv[1];
    int seed = atoi(argv[2]);
    srand(seed);

    set_edge_cases();
    run_one_time_edge_cases(SATPath);

    std::thread InputGenerationThread(generate_cnf_files);
    std::thread OutputProcessingThread(process_output);
    std::thread FuzzingThread1(fuzz, SATPath);
    std::thread FuzzingThread2(fuzz, SATPath);
    fuzz(SATPath);

    FuzzingThread1.join();
    FuzzingThread2.join();
    OutputProcessingThread.join();
    InputGenerationThread.join();
}

void process_output()
{
    while (true)
    {
        SaveToFileParams params = saveToFileQueue.pop();
        save_to_file(params.raw_error_output, params.CurrentInput);
    }
}

void fuzz(std::string SATPath)
{
    while (true)
    {
        InputCounterMutex.lock();
        CurrentCounterMutex.lock();
        long MaxInput = INPUT_COUNTER;
        long CurrentInput = CURRENT_COUNTER;
        CurrentCounterMutex.unlock();
        InputCounterMutex.unlock();
        // There is a chance this is vulnerable to race condition
        while (CurrentInput < MaxInput)
        {
            CurrentCounterMutex.lock();
            CurrentInput = CURRENT_COUNTER++;
            CurrentCounterMutex.unlock();
            std::string InputPath = "inputs/AUTOGEN_" + std::to_string(CurrentInput) + ".cnf";
            auto SUTProcess = subprocess::Popen({SATPath, InputPath}, subprocess::output(subprocess::PIPE), subprocess::error(subprocess::PIPE));
            execute(SUTProcess, CurrentInput);
            InputCounterMutex.lock();
            MaxInput = INPUT_COUNTER;
            InputCounterMutex.unlock();
        }
    }
}

void generate_cnf_files()
{
    int InputChoice;
    while (true)
    {
        InputChoice = rand() % 101;
        if (InputChoice < 10)
            generate_complex_correct_cnf_files();
        else if (InputChoice < 30)
            generate_simple_correct_cnf_files();
        else
            generate_trash_cnf_files();
        std::this_thread::sleep_for(std::chrono::milliseconds(15));
    }
}

void generate_complex_correct_cnf_files()
{
    InputCounterMutex.lock();
    std::string name = "inputs/AUTOGEN_" + std::to_string(INPUT_COUNTER++) + ".cnf";
    InputCounterMutex.unlock();
    std::ofstream file(name);

    file << generate_complex_correct_cnf() << "\n";
    file.close();
}

void generate_simple_correct_cnf_files()
{
    InputCounterMutex.lock();
    std::string name = "inputs/AUTOGEN_" + std::to_string(INPUT_COUNTER++) + ".cnf";
    InputCounterMutex.unlock();
    std::ofstream file(name);

    file << generate_simple_correct_cnf() << "\n";
    file.close();
}

void generate_trash_cnf_files()
{
    InputCounterMutex.lock();
    long counter = INPUT_COUNTER++;
    InputCounterMutex.unlock();
    std::string name = "inputs/AUTOGEN_" + std::to_string(counter) + ".cnf";
    std::ofstream file(name);

    file << generate_trash_cnf(counter) << "\n";
    file.close();
}

std::string generate_simple_correct_cnf()
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
    else if (randomChance < 50)
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

int pick(int from, int to)
{
    return (std::rand() % (to - from + 1)) + from;
}

std::string generate_complex_correct_cnf()
{
    std::stringstream output;

    int max_width;
    int nlayers;
    if (pick(1, 100) < 5)
    {
        max_width = pick(10, 70);
        nlayers = pick(1, 20);
    }
    else
    {
        max_width = pick(10, 40);
        nlayers = pick(1, 15);
    }
    std::vector<std::vector<int>> unused(nlayers);
    std::vector<int> width(nlayers), low(nlayers), high(nlayers), clauses(nlayers), nunused(nlayers);

    for (int i = 0; i < nlayers; i++)
    {
        width[i] = pick(10, max_width);
        low[i] = i ? high[i - 1] + 1 : 1;
        high[i] = low[i] + width[i] - 1;
        int m = width[i];
        if (i)
            m += width[i - 1];
        int n;
        if (pick(1, 100) < 20)
            n = (pick(150, 300) * m) / 100;
        else
            n = (pick(75, 150) * m) / 100;
        clauses[i] = n;

        nunused[i] = 2 * (high[i] - low[i] + 1);
        unused[i].resize(nunused[i]);
        int k = 0;
        for (int j = low[i]; j <= high[i]; j++)
        {
            unused[i][k++] = j;
            unused[i][k++] = -j;
        }
    }

    int n = 0;
    int m = high[nlayers - 1];
    std::vector<bool> used(m + 1, 0);

    for (int i = 0; i < nlayers; i++)
        n += clauses[i];

    output << "p cnf " << m << " " << n << std::endl;

    for (int i = 0; i < nlayers; i++)
    {
        for (int j = 0; j < clauses[i]; j++)
        {
            int l = 3;
            while (l < 100 && pick(1, 3) != 1)
                l++;

            for (int k = 0; k < l; k++)
            {
                int layer = i;
                int lit;
                while (layer && pick(3, 4) == 3)
                    layer--;
                if (nunused[layer] > 0)
                {
                    int o = nunused[layer] - 1;
                    int p = pick(0, o);
                    lit = unused[layer][p];
                    if (used[std::abs(lit)])
                        continue;
                    nunused[layer] = o;
                    if (p != o)
                        unused[layer][p] = unused[layer][o];
                }
                else
                {
                    lit = pick(low[layer], high[layer]);
                    if (used[std::abs(lit)])
                        continue;
                    int sign = (pick(1, 2) == 1) ? 1 : -1;
                    lit *= sign;
                }
                used[std::abs(lit)] = 1;
                output << lit << " ";
            }
            output << "0\n";
            used.assign(m + 1, 0);
        }
    }

    return output.str();
}

std::string generate_trash_cnf(long counter)
{
    std::string correct;
    if (pick(0, 100) < 20)
    {
        correct = generate_complex_correct_cnf();
    }
    else
    {
        correct = generate_simple_correct_cnf();
    }
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

void save_to_file(const char *raw_error_output, long CurrentInput)
{
    // ASYNC WRITE HERE
    std::string name = "fuzzed-tests/test_error_" + std::to_string(CurrentInput) + ".txt";
    std::string grep_content = "";

    for (size_t j = 0; j < REGEX_ERRORS; j++)
    {

        auto res = grep_output(raw_error_output, REGEX[j]);

        if (!res.isEmpty)
        {
            if (FilesCopied > 19)
            {
                Error *maxError = &(Errors[0]);
                int currentMax = 0;
                for (int k = 1; k < REGEX_ERRORS; k++)
                {
                    if (Errors[k].filename.size() > maxError->filename.size())
                    {
                        maxError = &(Errors[k]);
                        currentMax = k;
                    }
                }

                bool unique = true;
                std::string removedFile;
                while (unique)
                {
                    removedFile = maxError->filename.at(pick(0, maxError->filename.size() - 1));
                    bool tempUnique = false;
                    for (int r = 0; r < REGEX_ERRORS; r++)
                    {
                        if (errorInVector(r, removedFile) && Errors[r].filename.size()<=1)
                        {
                            tempUnique = true;
                        }
                    }
                    unique = tempUnique;
                };

                std::string command = "rm fuzzed-tests/" + removedFile;

                if (std::system(command.c_str()) == 0)
                {

                    for (int e = 0; e < REGEX_ERRORS; e++)
                    {

                        for (int f = 0; f < Errors[e].filename.size(); f++)
                        {
                            if (Errors[e].filename[f] == removedFile)
                            {
                                Errors[e].filename.erase(Errors[e].filename.begin() + f);
                            }
                        }
                    }

                    FilesCopied--;
                    std::cout << "File removed successfully. " << command.c_str() << std::endl;
                }
                else
                {
                    std::cerr << "File remove failed. " << command.c_str() << std::endl;
                }
            }

            grep_content += res.result + "\n";
            Errors[j].filename.push_back("AUTOGEN_" + std::to_string(CurrentInput) + ".cnf");
        }
    }

    if (grep_content != "")
    {

        std::string command = "cp inputs/AUTOGEN_" + std::to_string(CurrentInput) + ".cnf fuzzed-tests/AUTOGEN_" + std::to_string(CurrentInput) + ".cnf";

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

    for (int k = 0; k < REGEX_ERRORS; k++)
    {
        printf("Error: %d   Appeared: %lu times\n", k, Errors[k].filename.size());
        for (int l = 0; l < Errors[k].filename.size(); l++)
        {
            printf("File: %s\n", Errors[k].filename[l].c_str());
        }
    }
    printf("----------------------------------------------------\n");
}

bool errorInVector(int errorIndex, std::string &s)
{
    for (int i = 0; i < Errors[errorIndex].filename.size(); i++)
    {
        if (Errors[errorIndex].filename.at(i) == s)
        {
            return true;
        }
    }
    return false;
}

void execute(subprocess::Popen &SUTProcess, long CurrentInput)
{
    std::future<void> future = std::async(std::launch::async, [&]()
                                          { SUTProcess.wait(); });

    if (future.wait_for(std::chrono::seconds(15)) == std::future_status::timeout)
    {
        std::cerr << "SAT killed timeout reached -> ERROR: Infinite LOOP\n";
        SUTProcess.kill(15);
        saveToFileQueue.push({"SAT killed timeout reached -> ERROR: Infinite LOOP\n", CurrentInput});
    }
    else if (SUTProcess.retcode() != 0)
    {
        auto output = SUTProcess.communicate();
        saveToFileQueue.push({output.second.buf.data(), CurrentInput});
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
        execute(SUTProcess, i);
    }
    CURRENT_COUNTER = INPUT_COUNTER;
}