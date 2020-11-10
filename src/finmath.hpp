#pragma once

#include "jump/types.hpp"

#include <array>
#include <optional>
#include <string>
#include <tuple>
#include <vector>

namespace finmath {
/** The number of days in the investment period */
constexpr auto NB_DAYS = 365 * 4; // TODO

/** The number of "useful/important" assets */
constexpr auto NB_ASSETS = 40; // TODO

/** The value of an asset at a given day */
using asset_day_value_t = double;

/** The values for each day of an asset */
using asset_period_values_t = std::array<asset_day_value_t, NB_DAYS>;

/** The weight of the asset in the portfolio.
 * It is a percent of the invested capital */
using asset_weight_t = double;

/** An asset identification string */
using asset_id_t = const std::string &;

/** A investment portfolio */
struct portfolio_t {
  /** The invested assets and their weight in the portfolio */
  std::vector<std::tuple<asset_weight_t, asset_id_t>> investments;

  /** The capital that have not been invested */
  asset_weight_t not_invested_capital;
};

/** The matrix of covariance for each pair of assets */
using covariance_matrix_t =
    std::array<std::array<double, NB_ASSETS>, NB_ASSETS>;

/** Compute the covariance between two assets */
double compute_covariance(const asset_period_values_t &asset1_values,
                          const asset_period_values_t &asset2_values);

/** Return the volatility of the portfolio */
double compute_volatility(const covariance_matrix_t &cov_matrix,
                          const portfolio_t &portfolio);

/** Compute the sharpe of a portfolio */
double compute_sharpe(const covariance_matrix_t &cov_matrix,
                      const portfolio_t &portfolio);
} // namespace finmath
