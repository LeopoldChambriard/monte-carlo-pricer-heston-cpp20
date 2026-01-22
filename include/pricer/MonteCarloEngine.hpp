#pragma once

#include "Types.hpp"
#include "Payoff.hpp"
#include "PathGenerator.hpp"
#include <memory>

namespace pricer {

class MonteCarloEngine {
public:
    MonteCarloEngine(uint64_t num_paths = 500'000, uint32_t num_steps = 100, bool use_antithetic = true) noexcept
        : num_paths_(num_paths), num_steps_(num_steps), use_antithetic_(use_antithetic) {}

    [[nodiscard]] SimulationResult price_bs(
        const MarketData& market, double maturity, const IPayoff& payoff
    ) const;

    [[nodiscard]] SimulationResult price_heston(
        double spot, double rate, double maturity, const HestonParameters& params, const IPayoff& payoff
    ) const;

    [[nodiscard]] Greeks compute_greeks(
        const MarketData& market, double maturity, const IPayoff& payoff
    ) const;

private:
    uint64_t num_paths_;
    uint32_t num_steps_;
    bool use_antithetic_;
};

} // namespace pricer