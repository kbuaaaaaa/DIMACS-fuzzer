#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <future>
#include <sstream>
#include <climits>
#include <vector>
#include <regex>
#include <map>
#include "subprocess.hpp"
#include "HTTPRequest.hpp"


std::string NORM_HEADER = "p cnf 10 10\n";
std::string OVERFLOW_HEADER = "p cnf " + std::to_string(LLONG_MAX + 1) + " " + std::to_string(LLONG_MAX + 1) + "\n";
std::string RANDOM_HEADER = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~ \t\n\r\x0B\x0C";
std::string VALID_PUNCTUATION = "!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~ ";

std::vector<std::string> SPECIAL_INPUT = {
    "",
    "p cnf\n",
    OVERFLOW_HEADER,
    RANDOM_HEADER,
    NORM_HEADER,
    NORM_HEADER + "!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~" + "\n",
    NORM_HEADER + RANDOM_HEADER + "\n"
};

#pragma once
void execute(subprocess::Popen &SUTProcess);
//std::string random_gen_input(int seed);
void generate_cnf_files();
void generate_correct_cnf_files();
void generate_trash_cnf_files();
std::string generate_correct_cnf();
std::string generate_trash_cnf();


enum ErrorType {
    INTMIN_NEGATED,
    NULLPOINTER,
    SHIFT_ERROR,
    INTEGER_OVERFLOW,
    OTHER_ERROR,
    USE_AFTER_FREE,
    HEAP_BUFFER_OVERFLOW,
    STACK_BUFFER_OVERFLOW,
    GLOBAL_BUFFER_OVERFLOW,
    USE_AFTER_RETURN,
    USE_AFTER_SCOPE,
    ASAN_FAILED,
    MEMORY_LEAKS,
    UB_ERROR
};

std::string REGEX[] = {
    "SAT killed timeout",
    "^.*runtime error:.*negation",
    "^.*runtime error:.*null pointer",
    "^.*runtime error:.*shift",
    "^.*runtime error:.*integer",
    ".*runtime error:",
    "^==.*AddressSanitizer: heap-use-after-free",
    "^==.*AddressSanitizer: heap-buffer-overflow",
    "^==.*AddressSanitizer: stack-buffer-overflow",
    "^==.*AddressSanitizer: global-buffer-overflow",
    "^==.*AddressSanitizer: stack-use-after-return",
    "^==.*AddressSanitizer: stack-use-after-scope",
    "^==.*AddressSanitizer failed to",
    "^==.*LeakSanitizer: detected memory leaks",
    "^==.*UndefinedBehaviorSanitizer"
};


struct Error
{
    int count;
    std::string filename[20];
};


struct GrepReturn
{
    std::string result;
    bool isEmpty;
};

