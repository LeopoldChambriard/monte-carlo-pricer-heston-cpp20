#include <gtest/gtest.h>
#include "pricer/BlackScholesAnalytic.hpp"
#include "pricer/Payoff.hpp"
#include "pricer/MonteCarloEngine.hpp"
#include <cmath>

using namespace pricer;

TEST(MonteCarloTest, EuropeanCallConvergenceToBlackScholes) {
    MarketData market{
        .spot = 100.0,
        .rate = 0.05,
        .dividend = 0.0,
        .volatility = 0.20
    };
    const double strike = 100.0;
    const double maturity = 1.0;

    const double analytic_call = BlackScholesAnalytic::price(
        market.spot, strike, market.rate, market.dividend, market.volatility, maturity, OptionType::Call
    );

    MonteCarloEngine engine(500'000, 100, true);
    VanillaPayoff call_payoff(strike, OptionType::Call);
    SimulationResult result = engine.price_bs(market, maturity, call_payoff);

    EXPECT_NEAR(result.price, analytic_call, 3.0 * result.standard_error);
    EXPECT_LT(result.standard_error, 0.05);
}

TEST(MonteCarloTest, AsianOptionCheaperThanVanilla) {
    MarketData market{
        .spot = 100.0,
        .rate = 0.05,
        .dividend = 0.0,
        .volatility = 0.20
    };
    const double strike = 100.0;
    const double maturity = 1.0;

    MonteCarloEngine engine(200'000, 100, true);
    VanillaPayoff vanilla_call(strike, OptionType::Call);
    AsianPayoff asian_call(strike, OptionType::Call);

    const double vanilla_price = engine.price_bs(market, maturity, vanilla_call).price;
    const double asian_price = engine.price_bs(market, maturity, asian_call).price;

    EXPECT_LT(asian_price, vanilla_price);
}

TEST(MonteCarloTest, BarrierKnockOutIsDiscounted) {
    MarketData market{
        .spot = 100.0,
        .rate = 0.05,
        .dividend = 0.0,
        .volatility = 0.20
    };
    const double strike = 100.0;
    const double barrier = 120.0;
    const double maturity = 1.0;

    MonteCarloEngine engine(200'000, 100, true);
    VanillaPayoff vanilla_call(strike, OptionType::Call);
    BarrierUpAndOutPayoff barrier_call(strike, barrier, OptionType::Call);

    const double vanilla_price = engine.price_bs(market, maturity, vanilla_call).price;
    const double barrier_price = engine.price_bs(market, maturity, barrier_call).price;

    EXPECT_LT(barrier_price, vanilla_price);
    EXPECT_GT(barrier_price, 0.0);
}

TEST(HestonTest, FellerConditionAndPricing) {
    HestonParameters params{
        .v0 = 0.04,
        .kappa = 2.0,
        .theta = 0.04,
        .xi = 0.3,
        .rho = -0.7
    };
    EXPECT_TRUE(params.satisfies_feller());

    MonteCarloEngine engine(200'000, 100, false);
    VanillaPayoff call_payoff(100.0, OptionType::Call);

    SimulationResult res = engine.price_heston(100.0, 0.05, 1.0, params, call_payoff);
    EXPECT_GT(res.price, 0.0);
    EXPECT_FALSE(std::isnan(res.price));
}