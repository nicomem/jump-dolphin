#include "finmath.hpp"

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
                          const portfolio_t &portfolio,
                          const assets_day_values_t &start_values) {
  double vol = 0;
  for (const auto &[share1, asset1] : portfolio.investments) {
    double inner_sum = 0;
    for (const auto &[share2, asset2] : portfolio.investments) {
      inner_sum += share2 * start_values[asset2] * cov_matrix[asset1][asset2];
    }
    vol += share1 * start_values[asset1] * inner_sum;
  }
  return vol / (CAPITAL_START * CAPITAL_START);
}

double compute_sell_value(const portfolio_t &portfolio,
                          const assets_day_values_t &end_values) {
  double r = 0;
  for (const auto &[share, asset] : portfolio.investments) {
    r += share * end_values[asset];
  }
  return r;
}

double compute_sharpe(const covariance_matrix_t &cov_matrix,
                      const portfolio_t &portfolio,
                      const assets_day_values_t &start_values,
                      const assets_day_values_t &end_values) {
  // Compute the capital at the end of the period
  auto sell_value = compute_sell_value(portfolio, end_values);
  auto capital_end = sell_value + portfolio.not_invested_capital;

  // Compute the return of the investment and the return of a no-risk investment
  auto inv_return = capital_end / CAPITAL_START;

  auto vol = compute_volatility(cov_matrix, portfolio, start_values);
  return inv_return / vol;
}
} // namespace finmath
