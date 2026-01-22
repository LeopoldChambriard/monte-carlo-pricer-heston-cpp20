#include <benchmark/benchmark.h>
#include "pricer/Payoff.hpp"
#include "pricer/MonteCarloEngine.hpp"

using namespace pricer;

static void BM_BlackScholesMonteCarlo(benchmark::State& state) {
    MarketData market{
        .spot = 100.0,
        .rate = 0.05,
        .dividend = 0.0,
        .volatility = 0.20
    };
    VanillaPayoff payoff(100.0, OptionType::Call);
    MonteCarloEngine engine(100'000, 50, true);

    for (auto _ : state) {
        benchmark::DoNotOptimize(engine.price_bs(market, 1.0, payoff));
    }
    state.SetItemsProcessed(state.iterations() * 100'000);
}
BENCHMARK(BM_BlackScholesMonteCarlo)->Unit(benchmark::kMillisecond);

static void BM_HestonMonteCarlo(benchmark::State& state) {
    HestonParameters params{
        .v0 = 0.04,
        .kappa = 2.0,
        .theta = 0.04,
        .xi = 0.3,
        .rho = -0.7
    };
    VanillaPayoff payoff(100.0, OptionType::Call);
    MonteCarloEngine engine(50'000, 50, false);

    for (auto _ : state) {
        benchmark::DoNotOptimize(engine.price_heston(100.0, 0.05, 1.0, params, payoff));
    }
    state.SetItemsProcessed(state.iterations() * 50'000);
}
BENCHMARK(BM_HestonMonteCarlo)->Unit(benchmark::kMillisecond);

BENCHMARK_MAIN();