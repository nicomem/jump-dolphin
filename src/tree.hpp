#pragma once

#include "save_data.hpp"

#include <tuple>
#include <vector>

using nb_shares_t = unsigned;
using share_index_t = unsigned;
using compo_t = std::vector<std::tuple<nb_shares_t, share_index_t>>;
using sharpe_t = double;

struct TrucsInteressants {
  finmath::day_currency_rates_t start_rates;
  finmath::day_currency_rates_t end_rates;
  finmath::assets_day_values_t start_values;
  finmath::assets_day_values_t end_values;
  finmath::covariance_matrix_t cov_matrix;
  std::vector<finmath::nb_shares_t> nb_shares;
  std::vector<std::string> assets_id;
};

std::tuple<compo_t, sharpe_t>
max_compo_tree(const TrucsInteressants &trucs, compo_t &compo,
               double step = 0.1, sharpe_t parent_sharpe = -INFINITY);
