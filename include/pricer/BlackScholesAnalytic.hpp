#pragma once

#include "Types.hpp"

namespace pricer {

class BlackScholesAnalytic {
public:
    static double price(double spot, double strike, double rate, double div, double vol, double maturity, OptionType type) noexcept;
    static double delta(double spot, double strike, double rate, double div, double vol, double maturity, OptionType type) noexcept;
    static double gamma(double spot, double strike, double rate, double div, double vol, double maturity) noexcept;
    static double vega(double spot, double strike, double rate, double div, double vol, double maturity) noexcept;

private:
    static double normal_cdf(double x) noexcept;
    static double normal_pdf(double x) noexcept;
};

} // namespace pricer