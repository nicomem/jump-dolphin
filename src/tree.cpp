#include "tree.hpp"

// Min number of assets in portfolio
constexpr unsigned min_portfolio_size = 6; // 15;

// Max number of assets in portfolio
constexpr unsigned max_portfolio_size = 6; // 40;

// Min percent of the portfolio of a share
constexpr double min_share_percent = 0.01;

// Max percent of the portfolio of a share
constexpr double max_share_percent = 0.3;

// Min percent of stock percent assets in portfolio
constexpr double min_stock_percent = 0.5;

static double portolio_capital(const TrucsInteressants &trucs,
                               const compo_t &compo) {
  double r = 0;
  for (const auto &[nb_shares, i_asset] : compo) {
    r += nb_shares * trucs.start_values[i_asset];
  }
  return r;
}

/** Try to create the best portfolio from a combination of assets */
static sharpe_t fill_compo(const TrucsInteressants &trucs, compo_t &compo,
                           finmath::asset_period_values_t &shares_capital) {
  // Find the asset with the min capital
  double min_cap = INFINITY;
  for (const auto &[_nb_shares, i_asset] : compo) {
    min_cap = std::min(min_cap, trucs.assets_capital[i_asset]);
  }

  // Fill the portfolio
  auto max_cap = (max_share_percent / min_share_percent) * min_cap;
  for (auto &[nb_shares, i_asset] : compo) {
    nb_shares = max_cap / trucs.start_values[i_asset];
  }

  shares_capital.resize(compo.size());

  // Verify the portfolio
  auto compo_cap = portolio_capital(trucs, compo);
  for (const auto &[nb_shares, i_asset] : compo) {
    // Verify the portfolio conditions
    auto share_capital = nb_shares * trucs.start_values[i_asset];
    shares_capital.push_back(share_capital);

    auto true_ratio = share_capital / compo_cap;
    if (true_ratio < min_share_percent)
      return -INFINITY;

    // Should always be false, but just to be sure
    if (true_ratio > max_share_percent)
      return -INFINITY;
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
    vol = std::sqrt(vol / (compo_cap * compo_cap));
  }

  // Portfolio is valid, compute sharpe
  return (sell_value / compo_cap - 1) / (vol + 1e-8);
}

std::tuple<compo_t, sharpe_t>
max_compo_tree2(const TrucsInteressants &trucs,
                finmath::asset_period_values_t &shares_capital, compo_t &compo,
                share_index_t start_asset) {
  auto best_compo = compo_t();
  auto best_sharpe = -INFINITY;
  if (compo.size() >= min_portfolio_size) {
    auto compo_sharpe = fill_compo(trucs, compo, shares_capital);
    if (compo_sharpe > best_sharpe) {
      best_compo = compo;
      best_sharpe = compo_sharpe;
    }
  }

  if (compo.size() < max_portfolio_size) {
    for (auto i_asset = start_asset; i_asset < trucs.start_values.size();
         ++i_asset) {
      compo.emplace_back(-1, i_asset);

      auto [child_compo, child_sharpe] =
          max_compo_tree2(trucs, shares_capital, compo, i_asset + 1);
      if (child_sharpe > best_sharpe) {
        best_compo = child_compo;
        best_sharpe = child_sharpe;
      }

      compo.pop_back();
    }
  }

  return std::make_tuple(best_compo, best_sharpe);
}