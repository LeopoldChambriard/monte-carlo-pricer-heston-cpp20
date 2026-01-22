# Multi-Asset Monte Carlo Pricing Engine & Heston Calibration (C++20)

[![Language](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![Quantitative Finance](https://img.shields.io/badge/Derivatives-Exotic%20Pricing-purple.svg)](#)
[![Tests](https://img.shields.io/badge/Tests-GoogleTest%20Passed-brightgreen.svg)](#)
[![License](https://img.shields.io/badge/License-MIT-black.svg)](LICENSE)

A high-performance, multithreaded stochastic asset pricing engine implemented in modern C++20. 

Engineered for quantitative research and derivatives risk management, the framework prices vanilla and path-dependent exotic options (Asian, Barrier Up-and-Out) under standard Black-Scholes dynamics and **Heston Stochastic Volatility**, incorporating variance reduction techniques (Antithetic Variates) and finite-difference Greeks.

---

## 1. Mathematical Framework

### Asset Dynamics

#### 1. Black-Scholes Geometric Brownian Motion
Under the risk-neutral measure $\mathbb{Q}$:

$$dS_t = (r - q) S_t dt + \sigma S_t dW_t$$

#### 2. Heston Stochastic Volatility Model
Coupled stochastic differential equations (SDEs) governing asset price $S_t$ and variance $V_t$:

$$\begin{cases} dS_t = r S_t dt + \sqrt{V_t} S_t dW_t^S \\ dV_t = \kappa (\theta - V_t) dt + \xi \sqrt{V_t} dW_t^V \end{cases}$$

with instantaneous Brownian correlation:

$$d\langle W^S, W^V \rangle_t = \rho dt$$

* $\kappa$: Mean-reversion rate
* $\theta$: Long-term variance
* $\xi$: Volatility of volatility (vol-of-vol)
* $\rho$: Leverage effect correlation parameter

### Feller Condition & Discretization Scheme
To ensure the variance process $V_t$ remains strictly positive and does not hit zero, the Feller condition is verified:

$$2 \kappa \theta > \xi^2$$

Simulation paths employ the **Full Truncation Euler Scheme** (Lord, Koekkoek, and van Dijk, 2010) to eliminate negative variance instability during numerical integration:

$$\tilde{V}_{t+\Delta t} = \tilde{V}_t + \kappa (\theta - \tilde{V}_t^+) \Delta t + \xi \sqrt{\tilde{V}_t^+} \Delta W_t^V$$

$$V_{t+\Delta t} = \max(\tilde{V}_{t+\Delta t}, 0)$$

### Variance Reduction (Antithetic Variates)
Path simulation generates antithetic Gaussian pairs $(Z_k, -Z_k)$ to reduce Monte Carlo standard error $\sigma / \sqrt{N}$ without increasing random number generator overhead:

$$\hat{P}_{\text{antithetic}} = \frac{1}{2N} \sum_{i=1}^N \left( \Phi(S^{(+), i}) + \Phi(S^{(-), i}) \right)$$

---

## 2. Directory Layout

```text
monte-carlo-pricer-heston-cpp20/
├── CMakeLists.txt
├── README.md
├── include/
│   └── pricer/
│       ├── Types.hpp             # Market structures, Heston parameters & Greeks
│       ├── BlackScholesAnalytic.hpp # Closed-form European BS benchmark
│       ├── Payoff.hpp            # Polymorphic payoffs (Vanilla, Asian, Barrier)
│       ├── PathGenerator.hpp     # Vectorized SDE simulators (GBM & Heston)
│       └── MonteCarloEngine.hpp  # Multithreaded parallel engine (std::async)
├── src/
│   ├── BlackScholesAnalytic.cpp
│   ├── PathGenerator.cpp
│   └── MonteCarloEngine.cpp
├── tests/
│   └── test_pricer.cpp           # GoogleTest analytical convergence suite
└── benchmarks/
    └── bench_pricer.cpp          # Google Benchmark execution throughput