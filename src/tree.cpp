#include "tree.hpp"

// Min number of assets in portfolio
constexpr unsigned min_portfolio_size = 15; // 15;

// Max number of assets in portfolio
constexpr unsigned max_portfolio_size = 15; // 40;

// Min percent of the portfolio of a share
constexpr double min_share_percent = 0.01;

// Max percent of the portfolio of a share
constexpr double max_share_percent = 0.1;

// Min percent of stock percent assets in portfolio
constexpr double min_stock_percent = 0.5;

static nb_shares_t
max_shares_for_percent_capital(const TrucsInteressants &trucs,
                               double percent_share, share_index_t i_asset) {
  return trucs.nb_shares[i_asset] * percent_share;
}

static double share_capital(const TrucsInteressants &trucs,
                            nb_shares_t nb_shares, share_index_t i_asset) {
  return nb_shares * trucs.start_values[i_asset];
}

static double portolio_capital(const TrucsInteressants &trucs,
                               const compo_t &compo) {
  double r = 0;
  for (const auto &[nb_shares, i_asset] : compo) {
    r += share_capital(trucs, nb_shares, i_asset);
  }
  return r;
}

static bool is_valid_portfolio(const TrucsInteressants &trucs,
                               const compo_t &compo) {

  if (compo.size() < min_portfolio_size || compo.size() > max_portfolio_size) {
    return false;
  }

  auto portfolio_cap = portolio_capital(trucs, compo);
  for (const auto &[nb_shares, i_asset] : compo) {
    auto share_cap = share_capital(trucs, nb_shares, i_asset);
    auto ratio = share_cap / portfolio_cap;
    if (ratio < min_share_percent || ratio > max_share_percent) {
      return false;
    }
  }

  return true;
}

static double compute_sell_value(const TrucsInteressants &trucs,
                                 const compo_t &compo) {
  double r = 0;
  for (const auto &[nb_shares, i_asset] : compo) {
    r += nb_shares * trucs.end_values[i_asset];
  }
  return r;
}

static double compute_volatility(const TrucsInteressants &trucs,
                                 const compo_t &compo) {
  double vol = 0;

  auto portfolio_cap = portolio_capital(trucs, compo);
  for (const auto &[nb_shares1, asset1] : compo) {
    auto share1_cap = share_capital(trucs, nb_shares1, asset1);

    for (const auto &[nb_shares2, asset2] : compo) {
      auto share2_cap = share_capital(trucs, nb_shares2, asset2);

      vol += share1_cap * share2_cap * trucs.cov_matrix[asset1][asset2];
    }
  }

  return std::sqrt(vol / (portfolio_cap * portfolio_cap));
}

#include <iostream>

static double compute_sharpe(const TrucsInteressants &trucs,
                             const compo_t &compo) {
  // Compute the capital at the end of the period
  auto buy_value = portolio_capital(trucs, compo);
  auto sell_value = compute_sell_value(trucs, compo);

  // Compute the return of the investment and the return of a no-risk investment
  auto inv_return = sell_value / buy_value - 1;

  auto vol = compute_volatility(trucs, compo);
  // if (std::get<1>(compo[0]) == 259) {
  //   std::cout << std::get<0>(compo[0]) << '\n';
  //   std::cout << trucs.start_values[259] << " -> " << trucs.end_values[259]
  //             << '\n';
  //   std::cout << sell_value << ' ' << buy_value << ' ' << vol << '\n';
  // }

  return inv_return / (vol + 1e-8);
}

std::tuple<compo_t, sharpe_t> max_compo_tree(const TrucsInteressants &trucs,
                                             compo_t &compo, double step,
                                             sharpe_t parent_sharpe) {
  auto best_compo = compo;
  auto best_sharpe = parent_sharpe;

  auto i_parent = (compo.empty() ? -1 : std::get<1>(compo.back()));

  for (auto i_asset = i_parent + 1; i_asset < trucs.start_values.size();
       ++i_asset) {
    if (i_parent == -1) {
      std::clog << i_asset << '\n';
      std::clog << '#' << compo.size() << '\n';
    }
    for (auto share_percent = step; share_percent <= 1; share_percent += step) {
      nb_shares_t nb_shares =
          max_shares_for_percent_capital(trucs, share_percent, i_asset);
      if (nb_shares == 0) {
        continue;
      }

      compo.emplace_back(nb_shares, i_asset);

      sharpe_t sharpe = compute_sharpe(trucs, compo);
      if (sharpe > best_sharpe && is_valid_portfolio(trucs, compo)) {
        best_compo = compo;
        best_sharpe = sharpe;
      }

      bool is_interesting_sharpe = best_sharpe / sharpe < 2; // TODO
      if (!is_interesting_sharpe) {
        ;
      } else if (compo.size() >= max_portfolio_size) {
        ;
      } else {
        auto [child_compo, child_sharpe] =
            max_compo_tree(trucs, compo, step, best_sharpe);

        if (child_sharpe > best_sharpe &&
            is_valid_portfolio(trucs, child_compo)) {
          best_compo = child_compo;
          best_sharpe = child_sharpe;
        }
      }

      compo.pop_back();
    }
  }

  return std::make_tuple(best_compo, best_sharpe);
}
