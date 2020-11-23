#include "tree.hpp"

#include <chrono>
#include <iostream>

double portolio_capital(const TrucsInteressants &trucs, const compo_t &compo) {
  double r = 0;
  for (const auto &[nb_shares, i_asset] : compo) {
    r += nb_shares * trucs.start_values[i_asset];
  }
  return r;
}

sharpe_t compute_sharpe(const TrucsInteressants &trucs, const compo_t &compo,
                        finmath::asset_period_values_t &shares_capital) {
  shares_capital.resize(0);
  auto compo_cap = portolio_capital(trucs, compo);
  for (const auto &[nb_shares, i_asset] : compo) {
    auto share_capital = nb_shares * trucs.start_values[i_asset];
    shares_capital.push_back(share_capital);
  }

  double sell_value = 0;
  { // Compute sell value
    for (const auto &[nb_shares, i_asset] : compo) {
      sell_value += nb_shares * trucs.end_values[i_asset];
    }
  }

  double vol = 0;
  { // Compute volatility
    for (auto i = 0u; i < compo.size(); ++i) {
      const auto &asset1 = std::get<1>(compo[i]);

      auto tmp = 0;
      for (auto j = 0u; j < compo.size(); ++j) {
        const auto &asset2 = std::get<1>(compo[j]);

        tmp += shares_capital[j] * trucs.cov_matrix[asset1][asset2];
      }

      vol += shares_capital[i] * tmp;
    }
    vol = std::sqrt(vol) / compo_cap;
  }

  // Portfolio is valid, compute sharpe
  return (sell_value / compo_cap - 1) / (vol + 1e-8);
}

/** Try to create the best portfolio from a combination of assets */
static sharpe_t fill_compo(const TrucsInteressants &trucs, compo_t &compo,
                           finmath::asset_period_values_t &shares_capital) {
  // Find the asset with the min capital
  double min_cap = INFINITY;
  for (const auto &[_nb_shares, i_asset] : compo) {
    // std::cerr << trucs.assets_capital[i_asset] << '\n';
    min_cap = std::min(min_cap, trucs.assets_capital[i_asset]);
  }

  // Fill the portfolio
  auto max_cap = (max_share_percent / min_share_percent) * min_cap;
  for (auto &[nb_shares, i_asset] : compo) {
    nb_shares = max_cap / trucs.start_values[i_asset];
  }

  return compute_sharpe(trucs, compo, shares_capital);
}

/** Permute the composition
 * \return true if the composition have been permuted,
 * or false if the composition was already the last one
 */
static bool permute_compo(compo_t &compo, unsigned nb_assets,
                          unsigned nb_assets_perm) {
  auto i = 0u;
  for (; i < nb_assets_perm; ++i) {
    // Permute this index
    auto &c = std::get<1>(compo[compo.size() - i - 1]);
    ++c;

    // If this index has not reach the end, we can stop here
    // Else we need to permute the previous compo index
    if (c + i < nb_assets)
      break;
  }

  // If all index have been permuted and overset (==nb_assets)
  // The compo was the last permutation
  if (i == nb_assets_perm)
    return false;

  // Set the following index (that are currently == nb_assets)
  for (; i > 0; --i) {
    // Set the next index to the current + 1
    auto &c = std::get<1>(compo[compo.size() - i]);
    c = std::get<1>(compo[compo.size() - i - 1]) + 1;
  }

  return true;
}

std::tuple<compo_t, sharpe_t>
max_compo_tree2(const TrucsInteressants &trucs,
                finmath::asset_period_values_t &shares_capital, compo_t &compo,
                share_index_t start_asset) {
  auto best_compo = compo_t();
  auto best_sharpe = -INFINITY;

  // Create initial composition
  auto nb_assets_perm = max_portfolio_size - compo.size();
  for (auto i = start_asset; compo.size() < max_portfolio_size; ++i) {
    compo.emplace_back(-1, i);
  }

  // Not enough assets => return none found
  if (std::get<1>(compo.back()) >= trucs.assets_capital.size())
    return std::make_tuple(best_compo, best_sharpe);

  do {
    // auto t1 = std::chrono::system_clock::now();
    auto compo_sharpe = fill_compo(trucs, compo, shares_capital);

    // auto t2 = std::chrono::system_clock::now();
    // std::cerr << std::chrono::duration<double>(t2 - t1).count() << '\n';
    if (compo_sharpe > best_sharpe) {
      best_compo = compo;
      best_sharpe = compo_sharpe;
    }
  } while (permute_compo(compo, trucs.assets_capital.size(), nb_assets_perm));

  return std::make_tuple(best_compo, best_sharpe);
}