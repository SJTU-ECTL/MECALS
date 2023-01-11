#pragma once


#include <algorithm>
#include <bitset>
#include <boost/dynamic_bitset.hpp>
#include <boost/functional/hash.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/cpp_dec_float.hpp>
#include <boost/timer/progress_display.hpp>
#include <boost/timer/timer.hpp>
#include <boost/random.hpp>
#include <concepts>
#include <filesystem>
#include <iostream>
#include <limits.h>
#include <list>
#include <memory>
#include <queue>
#include <random>
#include <regex>
#include <set>
#include <sstream>
#include <stdint.h>
#include <termios.h>
#include <thread>
#include <tuple>
#include <unistd.h>
#include <unordered_map>
#include <unordered_set>


using ll = long long;
using ull = unsigned long;
using LLVec = std::vector<ll>;
using IntVec = std::vector<int>;
using DblVec = std::vector<double>;
using BigFlt = boost::multiprecision::cpp_dec_float_100;
using BigInt = boost::multiprecision::int512_t;


#define DEBUG