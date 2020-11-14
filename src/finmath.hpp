#pragma once

#include "jump/types.hpp"

#include <optional>
#include <string>
#include <tuple>
#include <vector>

namespace finmath {
/** The number of days in the investment period */
constexpr auto NB_DAYS = 365 * 4; // TODO

/** The capital of the portfolio at the start of the investment */
constexpr auto CAPITAL_START = 1; // TODO

/** The value of an asset at a given day */
using asset_day_value_t = double;

/** The values for each day of an asset */
using asset_period_values_t = std::vector<asset_day_value_t>;

/** The number of shares of the asset in the portfolio. */
using asset_share_t = unsigned;

/** The index of the asset (e.g. in the covariance matrix) */
using asset_index_t = unsigned;

/** A investment portfolio */
struct portfolio_t {
  /** The invested assets and their weight in the portfolio */
  std::vector<std::tuple<asset_share_t, asset_index_t>> investments;

  /** The capital that have not been invested */
  asset_share_t not_invested_capital;
};

/** The matrix of covariance for each pair of assets */
using covariance_matrix_t = std::vector<std::vector<double>>;

/** Value for each asset at a specific day */
using assets_day_values_t = std::vector<asset_day_value_t>;

/** Compute the covariance between two assets */
double compute_covariance(const asset_period_values_t &x_values,
                          asset_day_value_t x_mean,
                          const asset_period_values_t &y_values,
                          asset_day_value_t y_mean);

/** Return the volatility of the portfolio */
double compute_volatility(const covariance_matrix_t &cov_matrix,
                          const portfolio_t &portfolio,
                          const assets_day_values_t &start_values);

/** Compute the sell value of the portfolio */
double compute_sell_value(const portfolio_t &portfolio,
                          const assets_day_values_t &end_values);

/** Compute the sharpe of a portfolio */
double compute_sharpe(const covariance_matrix_t &cov_matrix,
                      const portfolio_t &portfolio,
                      const assets_day_values_t &start_values,
                      const assets_day_values_t &end_values);
} // namespace finmath
