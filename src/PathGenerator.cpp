#include "pricer/PathGenerator.hpp"
#include <cmath>
#include <algorithm>

namespace pricer {

void PathGenerator::generate_bs_path(
    std::vector<double>& path,
    double spot, double rate, double div, double vol, double maturity,
    uint32_t num_steps, std::mt19937_64& rng, bool antithetic,
    std::vector<double>* anti_path
) noexcept {
    const double dt = maturity / num_steps;
    const double drift = (rate - div - 0.5 * vol * vol) * dt;
    const double vol_sqrt_dt = vol * std::sqrt(dt);

    std::normal_distribution<double> dist(0.0, 1.0);

    path.resize(num_steps + 1);
    path[0] = spot;

    if (antithetic && anti_path) {
        anti_path->resize(num_steps + 1);
        (*anti_path)[0] = spot;
    }

    for (uint32_t i = 1; i <= num_steps; ++i) {
        const double z = dist(rng);
        path[i] = path[i - 1] * std::exp(drift + vol_sqrt_dt * z);

        if (antithetic && anti_path) {
            (*anti_path)[i] = (*anti_path)[i - 1] * std::exp(drift - vol_sqrt_dt * z);
        }
    }
}

void PathGenerator::generate_heston_path(
    std::vector<double>& spot_path,
    std::vector<double>& var_path,
    double spot, double rate, double maturity,
    const HestonParameters& params,
    uint32_t num_steps, std::mt19937_64& rng
) noexcept {
    const double dt = maturity / num_steps;
    const double sqrt_dt = std::sqrt(dt);
    const double sqrt_1_minus_rho2 = std::sqrt(1.0 - params.rho * params.rho);

    std::normal_distribution<double> dist(0.0, 1.0);

    spot_path.resize(num_steps + 1);
    var_path.resize(num_steps + 1);

    spot_path[0] = spot;
    var_path[0] = params.v0;

    for (uint32_t i = 1; i <= num_steps; ++i) {
        const double zv = dist(rng);
        const double zs = params.rho * zv + sqrt_1_minus_rho2 * dist(rng);

        // Troncature complète pour préserver la variance non négative
        const double v_plus = std::max(var_path[i - 1], 0.0);
        const double sqrt_v_plus = std::sqrt(v_plus);

        var_path[i] = var_path[i - 1] + params.kappa * (params.theta - v_plus) * dt 
                      + params.xi * sqrt_v_plus * sqrt_dt * zv;

        spot_path[i] = spot_path[i - 1] * std::exp((rate - 0.5 * v_plus) * dt 
                       + sqrt_v_plus * sqrt_dt * zs);
    }
}

} // namespace pricer