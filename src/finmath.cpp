#include "finmath.hpp"

namespace finmath {
double compute_covariance(const asset_period_values_t &asset1_values,
                          const asset_period_values_t &asset2_values) {
  // TODO
  (void)asset1_values;
  (void)asset2_values;
  return 0;
}

double compute_volatility(const covariance_matrix_t &cov_matrix,
                          const portfolio_t &portfolio) {
  // TODO
  (void)cov_matrix;
  (void)portfolio;
  return 0;
}

double compute_sharpe(const covariance_matrix_t &cov_matrix,
                      const portfolio_t &portfolio) {
  // TODO
  (void)cov_matrix;
  (void)portfolio;
  return 0;
}
} // namespace finmath
