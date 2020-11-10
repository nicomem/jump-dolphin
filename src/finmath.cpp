#include "finmath.hpp"

#include <cmath>

namespace finmath {
double compute_covariance(const asset_period_values_t &x_values,
                          asset_day_value_t x_mean,
                          const asset_period_values_t &y_values,
                          asset_day_value_t y_mean) {
  double cov = 0;
  for (size_t i = 0; i < NB_DAYS; ++i) {
    cov += (x_values[i] - x_mean) * (y_values[i] - y_mean);
  }
  return cov / NB_DAYS;
}

double compute_volatility(const covariance_matrix_t &cov_matrix,
                          const portfolio_t &portfolio) {
  double vol = 0;
  for (const auto &[w1, asset1] : portfolio.investments) {
    double inner_sum = 0;
    for (const auto &[w2, asset2] : portfolio.investments) {
      inner_sum += w2 * cov_matrix[asset1][asset2];
    }
    vol += w1 * inner_sum;
  }
  return vol;
}

double compute_sell_value(const portfolio_t &portfolio) {
  // TODO
  auto r = 0.;
  for (const auto &[w1, asset1] : portfolio.investments) {
    r += w1;
  }
  return r;
}

double compute_dividends(const portfolio_t &portfolio) {
  // TODO
  return 0;
}

double compute_sharpe(const covariance_matrix_t &cov_matrix,
                      const portfolio_t &portfolio) {
  // Compute the capital at the end of the period
  auto dividends_assets = compute_dividends(portfolio);
  auto sell_value = compute_sell_value(portfolio);
  auto capital_end =
      dividends_assets + sell_value + portfolio.not_invested_capital;

  // Compute the return of the investment and the return of a no-risk investment
  auto inv_return = std::pow(capital_end, 365. / NB_DAYS) - 1;
  auto no_risk_return = 0.; // TODO

  auto vol = compute_volatility(cov_matrix, portfolio);
  return (inv_return - no_risk_return) / vol;
}
} // namespace finmath
