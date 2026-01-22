#include "pricer/BlackScholesAnalytic.hpp"
#include <cmath>
#include <algorithm>
#include <numbers>

namespace pricer {

double BlackScholesAnalytic::normal_cdf(double x) noexcept {
    return 0.5 * std::erfc(-x / std::numbers::sqrt2);
}

double BlackScholesAnalytic::normal_pdf(double x) noexcept {
    return (1.0 / std::sqrt(2.0 * std::numbers::pi)) * std::exp(-0.5 * x * x);
}

double BlackScholesAnalytic::price(double spot, double strike, double rate, double div, double vol, double maturity, OptionType type) noexcept {
    if (maturity <= 0.0) {
        return (type == OptionType::Call) ? std::max(spot - strike, 0.0) : std::max(strike - spot, 0.0);
    }

    const double d1 = (std::log(spot / strike) + (rate - div + 0.5 * vol * vol) * maturity) / (vol * std::sqrt(maturity));
    const double d2 = d1 - vol * std::sqrt(maturity);

    const double df_r = std::exp(-rate * maturity);
    const double df_q = std::exp(-div * maturity);

    if (type == OptionType::Call) {
        return spot * df_q * normal_cdf(d1) - strike * df_r * normal_cdf(d2);
    } else {
        return strike * df_r * normal_cdf(-d2) - spot * df_q * normal_cdf(-d1);
    }
}

double BlackScholesAnalytic::delta(double spot, double strike, double rate, double div, double vol, double maturity, OptionType type) noexcept {
    const double d1 = (std::log(spot / strike) + (rate - div + 0.5 * vol * vol) * maturity) / (vol * std::sqrt(maturity));
    const double df_q = std::exp(-div * maturity);

    return (type == OptionType::Call) ? df_q * normal_cdf(d1) : df_q * (normal_cdf(d1) - 1.0);
}

double BlackScholesAnalytic::gamma(double spot, double strike, double rate, double div, double vol, double maturity) noexcept {
    const double d1 = (std::log(spot / strike) + (rate - div + 0.5 * vol * vol) * maturity) / (vol * std::sqrt(maturity));
    const double df_q = std::exp(-div * maturity);

    return (df_q * normal_pdf(d1)) / (spot * vol * std::sqrt(maturity));
}

double BlackScholesAnalytic::vega(double spot, double strike, double rate, double div, double vol, double maturity) noexcept {
    const double d1 = (std::log(spot / strike) + (rate - div + 0.5 * vol * vol) * maturity) / (vol * std::sqrt(maturity));
    const double df_q = std::exp(-div * maturity);

    return spot * df_q * std::sqrt(maturity) * normal_pdf(d1);
}

} // namespace pricer
