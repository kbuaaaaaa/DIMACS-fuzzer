#!/usr/bin/env bash
set -x # Display every executed instruction
set -e # Fail script as soon as instruction fails

if [ -d ./fuzzed-tests ]; then
    echo "Directory exists"
    rm -rf fuzzed-tests
fi

rm -rf inputs/AUTOGEN*
mkdir fuzzed-tests

g++ ./fuzzer/main.cpp -pthread -O3 -o fuzz-sat
# Implement your build script here...
# You can assume this runs on an environment similar to a lab-machine.
#
# This can involve installing language compilers, interpreters and runtimes,
# compiling source code with available tools or do nothing if your
# fuzzer is written in an interpreted language for which the
# interpreter is already available on a standard lab-machine. If
# needed this script should set necessary permissions to make your
# build outputs executable.  Once this script as run, we assume the
# presence of a ./fuzz-sat executable that can be invoked as
# ./fuzz-sat /path/to/SUT /path/to/inputs mode seed as outlined in the
# specification.

exit 0
