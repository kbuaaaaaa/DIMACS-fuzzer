#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <future>
#include "subprocess.hpp"
#include <sstream>
#include <climits>
#include <vector>
#include <regex>
#include <map>


std::string OVERFLOW_LITERAL = std::to_string(LLONG_MAX + 1);
std::string ALPHANUMERIC = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
std::string VALID_PUNCTUATION = "!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~ ";
std::vector ESCAPE_CHARS = {'\a', '\b', '\f', '\n', '\r', '\t', '\v', '\\', '\'', '\"', '\?', '\x0B','\x0C'};
std::vector BITWISE_OPERATOR = {"&&", "||", "^", "<<", ">>", "-"};
std::vector RANDOM_ALL = {"&&", "||", "^", "<<", ">>", "-", "\r", "\t", "\v", "\\", "\"", "\x0B", "#", "$", "%", "+", "-", "a", "b", "c", "d", "e", "f"};


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

