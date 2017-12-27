#!/usr/bin/env bash
g++ -std=c++11 lab3_utils.cpp lab3_parser.cpp lab3.cpp -o lab3
g++ -std=c++11 lab3_utils.cpp lab3_parser.cpp ssa_to_3add.cpp -o ssa_to_3add
# A script that builds your compiler.
