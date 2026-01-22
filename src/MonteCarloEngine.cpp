#include "pricer/MonteCarloEngine.hpp"
#include <chrono>
#include <thread>
#include <future>
#include <cmath>
#include <algorithm>
#include <utility>

namespace pricer {

SimulationResult MonteCarloEngine::price_bs(
    const MarketData& market, double maturity, const IPayoff& payoff
) const {
    const auto start_time = std::chrono::high_resolution_clock::now();

    const unsigned int num_threads = std::max(1u, std::thread::hardware_concurrency());
    const uint64_t paths_per_thread = num_paths_ / num_threads;

    auto worker = [this, &market, maturity, &payoff](uint64_t paths, uint64_t seed) -> std::pair<double, double> {
        std::mt19937_64 rng(seed);
        std::vector<double> path;
        std::vector<double> anti_path;

        double sum_payoff = 0.0;
        double sum_payoff_sq = 0.0;

        const uint64_t loops = use_antithetic_ ? (paths / 2) : paths;

        for (uint64_t i = 0; i < loops; ++i) {
            PathGenerator::generate_bs_path(
                path, market.spot, market.rate, market.dividend, market.volatility,
                maturity, num_steps_, rng, use_antithetic_, &anti_path
            );

            if (use_antithetic_) {
                const double p1 = payoff.evaluate_path(path);
                const double p2 = payoff.evaluate_path(anti_path);
                const double p_avg = 0.5 * (p1 + p2);

                sum_payoff += p1 + p2;
                sum_payoff_sq += p_avg * p_avg * 2.0;
            } else {
                const double p = payoff.evaluate_path(path);
                sum_payoff += p;
                sum_payoff_sq += p * p;
            }
        }
        return {sum_payoff, sum_payoff_sq};
    };

    std::vector<std::future<std::pair<double, double>>> futures;
    for (unsigned int t = 0; t < num_threads; ++t) {
        futures.push_back(std::async(std::launch::async, worker, paths_per_thread, 1337 + t * 997));
    }

    double total_sum = 0.0;
    double total_sum_sq = 0.0;
    for (auto& f : futures) {
        auto [s, sq] = f.get();
        total_sum += s;
        total_sum_sq += sq;
    }

    const uint64_t total_paths = paths_per_thread * num_threads;
    const double df = std::exp(-market.rate * maturity);
    const double mean = total_sum / total_paths;
    const double discounted_price = df * mean;

    const double variance = (total_sum_sq / total_paths) - (mean * mean);
    const double standard_error = df * std::sqrt(std::max(0.0, variance) / total_paths);

    const auto end_time = std::chrono::high_resolution_clock::now();
    const double elapsed_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();

    return SimulationResult{
        .price = discounted_price,
        .standard_error = standard_error,
        .confidence_interval_low = discounted_price - 1.96 * standard_error,
        .confidence_interval_high = discounted_price + 1.96 * standard_error,
        .paths_computed = total_paths,
        .compute_time_ms = elapsed_ms
    };
}

SimulationResult MonteCarloEngine::price_heston(
    double spot, double rate, double maturity, const HestonParameters& params, const IPayoff& payoff
) const {
    const auto start_time = std::chrono::high_resolution_clock::now();

    const unsigned int num_threads = std::max(1u, std::thread::hardware_concurrency());
    const uint64_t paths_per_thread = num_paths_ / num_threads;

    auto worker = [this, spot, rate, maturity, &params, &payoff](uint64_t paths, uint64_t seed) -> std::pair<double, double> {
        std::mt19937_64 rng(seed);
        std::vector<double> spot_path;
        std::vector<double> var_path;

        double sum_payoff = 0.0;
        double sum_payoff_sq = 0.0;

        for (uint64_t i = 0; i < paths; ++i) {
            PathGenerator::generate_heston_path(spot_path, var_path, spot, rate, maturity, params, num_steps_, rng);
            const double p = payoff.evaluate_path(spot_path);
            sum_payoff += p;
            sum_payoff_sq += p * p;
        }
        return {sum_payoff, sum_payoff_sq};
    };

    std::vector<std::future<std::pair<double, double>>> futures;
    for (unsigned int t = 0; t < num_threads; ++t) {
        futures.push_back(std::async(std::launch::async, worker, paths_per_thread, 4242 + t * 773));
    }

    double total_sum = 0.0;
    double total_sum_sq = 0.0;
    for (auto& f : futures) {
        auto [s, sq] = f.get();
        total_sum += s;
        total_sum_sq += sq;
    }

    const uint64_t total_paths = paths_per_thread * num_threads;
    const double df = std::exp(-rate * maturity);
    const double mean = total_sum / total_paths;
    const double discounted_price = df * mean;

    const double variance = (total_sum_sq / total_paths) - (mean * mean);
    const double standard_error = df * std::sqrt(std::max(0.0, variance) / total_paths);

    const auto end_time = std::chrono::high_resolution_clock::now();
    const double elapsed_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();

    return SimulationResult{
        .price = discounted_price,
        .standard_error = standard_error,
        .confidence_interval_low = discounted_price - 1.96 * standard_error,
        .confidence_interval_high = discounted_price + 1.96 * standard_error,
        .paths_computed = total_paths,
        .compute_time_ms = elapsed_ms
    };
}

Greeks MonteCarloEngine::compute_greeks(
    const MarketData& market, double maturity, const IPayoff& payoff
) const {
    const double h_spot = market.spot * 0.01;
    const double h_vol  = 0.01;

    MarketData m_up = market;   m_up.spot += h_spot;
    MarketData m_down = market; m_down.spot -= h_spot;
    MarketData m_base = market;
    MarketData m_vol_up = market; m_vol_up.volatility += h_vol;

    const double p_up   = price_bs(m_up, maturity, payoff).price;
    const double p_down = price_bs(m_down, maturity, payoff).price;
    const double p_base = price_bs(m_base, maturity, payoff).price;
    const double p_vol  = price_bs(m_vol_up, maturity, payoff).price;

    const double delta = (p_up - p_down) / (2.0 * h_spot);
    const double gamma = (p_up - 2.0 * p_base + p_down) / (h_spot * h_spot);
    const double vega  = (p_vol - p_base) / h_vol;

    return Greeks{
        .delta = delta,
        .gamma = gamma,
        .vega = vega
    };
}

} // namespace pricer