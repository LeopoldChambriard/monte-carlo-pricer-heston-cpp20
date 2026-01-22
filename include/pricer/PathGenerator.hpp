#pragma once

#include "Types.hpp"
#include <random>
#include <vector>

namespace pricer {

class PathGenerator {
public:
    static void generate_bs_path(
        std::vector<double>& path,
        double spot, double rate, double div, double vol, double maturity,
        uint32_t num_steps, std::mt19937_64& rng, bool antithetic,
        std::vector<double>* anti_path = nullptr
    ) noexcept;

    static void generate_heston_path(
        std::vector<double>& spot_path,
        std::vector<double>& var_path,
        double spot, double rate, double maturity,
        const HestonParameters& params,
        uint32_t num_steps, std::mt19937_64& rng
    ) noexcept;
};

} // namespace pricer