#pragma once
#include "algorithm.h"
#include <iostream>
#include <mutex>
#include <future>
#include <vector>
#include <string>
#include <functional>
#include <array>

static constexpr auto DIMENSIONS = Weights::SIZE;
using Point = std::array<double, DIMENSIONS>;
std::pair<Point, double> particle_swarm(double min, double max, std::function<double(Point, std::string)> calculate_value, std::vector<std::string>& input_files, const int threads, const int groups);