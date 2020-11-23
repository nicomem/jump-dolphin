#include "stochastic.hpp"
#include "check.hpp"

#include <iostream>
#include <random>

sharpe_t compute_sharpe_init_chache(const TrucsInteressants &trucs,
                                    const compo_t &compo, SharpeCache &cache) {
  // Initialize the vectors
  auto compo_size = compo.size();
  cache.buy_values.resize(0);
  cache.buy_values.reserve(compo_size);

  // Set the start values & capital
  cache.start_capital = 0;
  for (const auto &[nb_shares, i_asset] : compo) {
    cache.start_capital += cache.buy_values.emplace_back(
        (double)nb_shares * trucs.start_values[i_asset]);
  }

  // Set the end values & capital
  cache.end_capital = 0;
  for (const auto &[nb_shares, i_asset] : compo) {
    cache.end_capital += (double)nb_shares * trucs.end_values[i_asset];
  }

  double vol = 0;
  for (auto i = 0u; i < compo_size; ++i) {
    auto asset1 = std::get<1>(compo[i]);
    const auto &cov_vec = trucs.cov_matrix[asset1];

    double subvol = 0;
    for (auto j = 0u; j < compo_size; ++j) {
      auto asset2 = std::get<1>(compo[j]);

      subvol += cache.buy_values[j] * cov_vec[asset2];
    }
    vol += cache.buy_values[i] * subvol;
  }

  // Compute the sharpe
  return (cache.end_capital - cache.start_capital) / std::sqrt(vol);
}

sharpe_t recompute_sharpe(SharpeCache &cache, unsigned i_compo_changed,
                          double dshares, bool only_update_cache) {
  auto &[shares, i_asset] = cache.compo[i_compo_changed];
  shares += dshares;

  // Update start & end capital
  auto dbuy_value = (double)dshares * cache.trucs.start_values[i_asset];
  cache.buy_values[i_compo_changed] += dbuy_value;
  cache.start_capital += dbuy_value;

  cache.end_capital += dshares * cache.trucs.end_values[i_asset];

  if (only_update_cache || !check_compo_cache(cache))
    return -INFINITY;

  auto compo_size = cache.compo.size();
  double vol = 0;
  for (auto i = 0u; i < compo_size; ++i) {
    auto asset1 = std::get<1>(cache.compo[i]);
    const auto &cov_vec = cache.trucs.cov_matrix[asset1];

    double subvol = 0;
    for (auto j = 0u; j < compo_size; ++j) {
      auto asset2 = std::get<1>(cache.compo[j]);

      subvol += cache.buy_values[j] * cov_vec[asset2];
    }
    vol -= cache.buy_values[i] * subvol;
  }

  // Compute the sharpe
  return (cache.end_capital - cache.start_capital) / std::sqrt(vol);
}

bool check_compo_cache(const SharpeCache &cache) {
  // Verify the %NAV
  for (auto buy_value : cache.buy_values) {
    auto true_ratio = buy_value / cache.start_capital;
    if (true_ratio < min_share_percent || true_ratio > max_share_percent) {
      return false;
    }
  }

  // TODO: Check stock proportion
  return true;
}

std::tuple<compo_t, sharpe_t>
optimize_compo_stochastic(const TrucsInteressants &trucs, compo_t compo,
                          std::function<double(const compo_t)> get_sharpe) {
  constexpr auto n_iter = 1'000u;

  std::random_device rd;
  std::mt19937 gen(rd());

  auto cache = SharpeCache(trucs, compo);
  auto best_sharpe = compute_sharpe_init_chache(trucs, compo, cache);
  best_sharpe = get_sharpe(compo);

  auto dshare = std::normal_distribution<double>(0, 1);
  auto dasset = std::uniform_int_distribution<unsigned>(0, compo.size() - 1);
  for (auto _i = 0u; _i < n_iter; ++_i) {
    int dx;
    do {
      dx = 25 * dshare(gen);
    } while (dx == 0);

    share_index_t i = dasset(gen);

    // Clamp the share modifier to be in bound
    auto &[shares, i_asset] = compo[i];
    dx = std::max<int>(-shares + 1, dx);
    dx = std::min<int>(dx, trucs.nb_shares[i_asset] - shares);

    auto sharpe_opt = recompute_sharpe(cache, i, dx, false);
    // Set best sharpe if better sharpe and still valid
    if (sharpe_opt > 0) {
      auto sharpe = get_sharpe(compo);
      if (sharpe > best_sharpe) {
        best_sharpe = sharpe;
      } else {
        // Undo action
        recompute_sharpe(cache, i, -dx, true);
      }
    } else {
      // Undo action
      recompute_sharpe(cache, i, -dx, true);
    }
  }

  return std::make_tuple(compo, best_sharpe);
}

static void swap_low_capital_ratio(const TrucsInteressants &trucs,
                                   compo_t &compo, std::mt19937 &gen,
                                   std::vector<bool> &assets_selected) {
  // Build vector of selected assets
  assets_selected.resize(trucs.assets_capital.size());
  std::fill(assets_selected.begin(), assets_selected.end(), false);

  auto start_capital = 0;
  double min_capital = INFINITY;
  double max_capital = -INFINITY;
  for (const auto &[shares, i_asset] : compo) {
    assets_selected[i_asset] = true;
    auto capital = (double)shares * trucs.start_values[i_asset];
    start_capital += capital;

    min_capital = std::min(min_capital, capital);
    max_capital = std::max(max_capital, capital);
  }

  auto cap_limit = std::normal_distribution<double>(0, 1)(gen) *
                   (max_capital - min_capital) / 2;
  auto asset_gen = std::uniform_int_distribution<unsigned>(0, compo.size() - 1);

  // Swap those with low capital ratios
  for (auto &[shares, i_asset] : compo) {
    auto capital = ((double)shares * trucs.start_values[i_asset]);
    if (capital >= cap_limit)
      continue;

    // Select random new asset
    unsigned new_asset;
    do {
      new_asset = asset_gen(gen);
    } while (assets_selected[new_asset]);

    // Swap
    assets_selected[i_asset] = false;
    assets_selected[new_asset] = true;
    i_asset = new_asset;
  }

  // Reset shares to respect the rules
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
}

compo_t
find_best_compo_stochastic(const TrucsInteressants &trucs, compo_t compo,
                           std::function<double(const compo_t)> get_sharpe) {
  constexpr auto nb_iter = 25u;

  std::random_device rd;
  std::mt19937 gen(rd());
  auto assets_selected = std::vector<bool>();

  // Reset shares to respect the rules
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

  auto [best_compo, _sharpe] =
      optimize_compo_stochastic(trucs, compo, get_sharpe);
  auto best_sharpe = get_sharpe(best_compo);
  std::clog << "Start compute sharpe: " << best_sharpe << '\n';
  for (auto _i = 0u; _i < nb_iter; ++_i) {
    // Swap those with low capital ratios with random ones
    compo = best_compo;
    swap_low_capital_ratio(trucs, compo, gen, assets_selected);

    auto [new_compo, _sharpe] =
        optimize_compo_stochastic(trucs, compo, get_sharpe);
    auto new_sharpe = get_sharpe(new_compo);
    if (new_sharpe > best_sharpe) {
      std::clog << "New best compute sharpe: " << new_sharpe << '\n';
      best_compo = new_compo;
      best_sharpe = new_sharpe;
    }
  }

  std::clog << "Final compute sharpe: " << best_sharpe << '\n';
  return best_compo;
}