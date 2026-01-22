#pragma once

#include "Types.hpp"
#include <numeric>
#include <algorithm>
#include <span>

namespace pricer {

class IPayoff {
public:
    virtual ~IPayoff() = default;
    [[nodiscard]] virtual double evaluate_path(std::span<const double> path) const noexcept = 0;
};

// Option Européenne standard
class VanillaPayoff final : public IPayoff {
public:
    VanillaPayoff(double strike, OptionType type) noexcept : strike_(strike), type_(type) {}

    [[nodiscard]] double evaluate_path(std::span<const double> path) const noexcept override {
        const double terminal_price = path.back();
        return (type_ == OptionType::Call) ? std::max(terminal_price - strike_, 0.0)
                                           : std::max(strike_ - terminal_price, 0.0);
    }

private:
    double strike_;
    OptionType type_;
};

// Option Asiatique à moyenne arithmétique continue
class AsianPayoff final : public IPayoff {
public:
    AsianPayoff(double strike, OptionType type) noexcept : strike_(strike), type_(type) {}

    [[nodiscard]] double evaluate_path(std::span<const double> path) const noexcept override {
        const double avg_price = std::accumulate(path.begin(), path.end(), 0.0) / static_cast<double>(path.size());
        return (type_ == OptionType::Call) ? std::max(avg_price - strike_, 0.0)
                                           : std::max(strike_ - avg_price, 0.0);
    }

private:
    double strike_;
    OptionType type_;
};

// Option Barrière Up-and-Out Call
class BarrierUpAndOutPayoff final : public IPayoff {
public:
    BarrierUpAndOutPayoff(double strike, double barrier, OptionType type) noexcept
        : strike_(strike), barrier_(barrier), type_(type) {}

    [[nodiscard]] double evaluate_path(std::span<const double> path) const noexcept override {
        for (double price : path) {
            if (price >= barrier_) return 0.0;
        }
        const double terminal_price = path.back();
        return (type_ == OptionType::Call) ? std::max(terminal_price - strike_, 0.0)
                                           : std::max(strike_ - terminal_price, 0.0);
    }

private:
    double strike_;
    double barrier_;
    OptionType type_;
};

} // namespace pricer