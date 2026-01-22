#pragma once

#include <cstdint>
#include <vector>
#include <string>

namespace pricer {

enum class OptionType : uint8_t {
    Call = 0,
    Put  = 1
};

struct MarketData {
    double spot{100.0};
    double rate{0.05};       // Taux sans risque r
    double dividend{0.0};    // Dividende continu q
    double volatility{0.20}; // Volatilité constante sigma
};

// Paramètres de volatilité stochastique de Heston
struct HestonParameters {
    double v0{0.04};     // Variance initiale
    double kappa{2.0};   // Vitesse de retour à la moyenne
    double theta{0.04};  // Variance moyenne à long terme
    double xi{0.3};      // Volatilité de la variance (vol-of-vol)
    double rho{-0.7};    // Corrélation brownienne d<W_S, W_V> = rho * dt

    // Condition de Feller : 2 * kappa * theta > xi^2 (garantit v_t > 0)
    [[nodiscard]] bool satisfies_feller() const noexcept {
        return (2.0 * kappa * theta) > (xi * xi);
    }
};

struct SimulationResult {
    double price{0.0};
    double standard_error{0.0};
    double confidence_interval_low{0.0};
    double confidence_interval_high{0.0};
    uint64_t paths_computed{0};
    double compute_time_ms{0.0};
};

struct Greeks {
    double delta{0.0};
    double gamma{0.0};
    double vega{0.0};
    double theta{0.0};
    double rho{0.0};
};

} // namespace pricer