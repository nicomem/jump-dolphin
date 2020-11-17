#pragma once

#include "save_data.hpp"

#include <tuple>
#include <vector>

using nb_shares_t = unsigned;
using share_index_t = unsigned;
using compo_t = std::vector<std::tuple<nb_shares_t, share_index_t>>;
using sharpe_t = double;

// Min number of assets in portfolio
constexpr unsigned min_portfolio_size = 40; // 15;

// Max number of assets in portfolio
constexpr unsigned max_portfolio_size = 40; // 40;

// Min percent of the portfolio of a share
constexpr double min_share_percent = 0.01;

// Max percent of the portfolio of a share
constexpr double max_share_percent = 0.1;

// Min percent of stock percent assets in portfolio
constexpr double min_stock_percent = 0.5;

struct TrucsInteressants {
  finmath::assets_day_values_t start_values;
  finmath::assets_day_values_t end_values;
  finmath::covariance_matrix_t cov_matrix;
  std::vector<finmath::nb_shares_t> nb_shares;
  std::vector<std::string> assets_id;

  std::vector<double> assets_capital;
};

/** Compute the portfolio capital at the start of the investment */
double portolio_capital(const TrucsInteressants &trucs, const compo_t &compo);

std::tuple<compo_t, sharpe_t>
max_compo_tree2(const TrucsInteressants &trucs,
                finmath::asset_period_values_t &shares_capital, compo_t &compo,
                share_index_t start_asset = 0);