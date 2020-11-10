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

double compute_sharpe(const covariance_matrix_t &cov_matrix,
                      const portfolio_t &portfolio) {
  // TODO
  (void)cov_matrix;
  (void)portfolio;
  return 0;
}
} // namespace finmath
