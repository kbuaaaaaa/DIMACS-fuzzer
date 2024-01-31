#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <future>
#include <sstream>
#include <climits>
#include <vector>
#include <regex>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <map>
#include "subprocess.hpp"
#include "HTTPRequest.hpp"


std::string OVERFLOW_LITERAL = std::to_string(LLONG_MAX + 1);
std::string ALPHANUMERIC = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
std::string VALID_PUNCTUATION = "!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~ ";
std::vector ESCAPE_CHARS = {'\a', '\b', '\f', '\n', '\r', '\t', '\v', '\\', '\'', '\"', '\?', '\x0B','\x0C'};
std::vector BITWISE_OPERATOR = {"&&", "||", "^", "<<", ">>", "-", "0&&1", "1||1", "0^1", "1<<9", "999>>1"};
std::vector RANDOM_ALL = {"&&", "||", "^", "<<", ">>", "-", "\r", "\t", "\v", "\\", "\"", "\x0B", "#", "$", "%", "+", "-", "a", "b", "c", "d", "e", "f", "\x7F\x45\x4C\x46", "\x89\x50\x4E\x47\x0D\x0A\x1A\x0A", "\xFF\xD8\xFF", "\x1F\x8B", "\x25\x50\x44\x46", "\x50\x4B\x03\x04"};
std::vector RANDOM_BYTES = {"\x7F\x45\x4C\x46", "\x89\x50\x4E\x47\x0D\x0A\x1A\x0A", "\xFF\xD8\xFF", "\x1F\x8B", "\x25\x50\x44\x46", "\x50\x4B\x03\x04"};


#pragma once
void execute(subprocess::Popen &SUTProcess, long CurrentInput);
//std::string random_gen_input(int seed);
void set_edge_cases();
void run_one_time_edge_cases(std::string SATPath);
void generate_cnf_files();
void generate_complex_correct_cnf_files();
void generate_simple_correct_cnf_files();
void generate_trash_cnf_files();
std::string generate_complex_correct_cnf();
std::string generate_simple_correct_cnf();
std::string generate_trash_cnf(long counter);
void fuzz(std::string SATPath);
void save_to_file(const char *raw_error_output, long CurrentInput);
void process_output();
bool errorInVector(int errorIndex, std::string &s);

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

const int REGEX_ERRORS = 18;
std::string REGEX[] = {
    "SAT killed timeout",
    "^.*runtime error:.*negation",
    "^.*runtime error:.*null pointer",
    "^.*runtime error:.*shift",
    "^.*runtime error:.*integer",
    "^.*runtime error: variable length",
    "^.*runtime error: load of misaligned address",
    "^==.*AddressSanitizer: SEGV",
    "^==.*AddressSanitizer: requested allocation size",
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
    std::vector<std::string> filename;
};


struct GrepReturn
{
    std::string result;
    bool isEmpty;
};


template <typename T>
class ThreadSafeQueue {
private:
    std::queue<T> queue_;
    std::mutex mutex_;
    std::condition_variable cond_;

public:
    void push(T value) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(std::move(value));
        cond_.notify_one();
    }

    T pop() {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_.wait(lock, [this]{ return !queue_.empty(); });
        T value = std::move(queue_.front());
        queue_.pop();
        return value;
    }
};

struct SaveToFileParams {
    const char *raw_error_output;
    long CurrentInput;
};