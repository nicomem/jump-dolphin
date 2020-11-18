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
    cache.start_capital +=
        cache.buy_values.emplace_back(nb_shares * trucs.start_values[i_asset]);
  }

  // Set the end values & capital
  cache.end_capital = 0;
  for (const auto &[nb_shares, i_asset] : compo) {
    cache.end_capital += nb_shares * trucs.end_values[i_asset];
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
  auto dbuy_value = dshares * cache.trucs.start_values[i_asset];
  cache.buy_values[i_compo_changed] += dbuy_value;
  cache.start_capital += dbuy_value;

  cache.end_capital += dshares * cache.trucs.end_values[i_asset];

  if (only_update_cache)
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
    vol += cache.buy_values[i] * subvol;
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

compo_t optimize_compo_stochastic(const TrucsInteressants &trucs,
                                  compo_t compo) {
  constexpr auto n_iter = 1'000'000u;

  std::random_device rd;
  std::mt19937 gen(rd());

  auto cache = SharpeCache(trucs, compo);
  auto best_sharpe = compute_sharpe_init_chache(trucs, compo, cache);
  std::cerr << "Program sharpe before opti: " << best_sharpe << '\n';

  auto dshare = std::normal_distribution<double>(0, 1);
  auto dasset = std::uniform_int_distribution<unsigned>(0, compo.size() - 1);
  for (auto _i = 0u; _i < n_iter; ++_i) {
    int dx;
    do {
      dx = 10 * dshare(gen);
    } while (dx == 0);

    share_index_t i = dasset(gen);

    // Clamp the share modifier to be in bound
    auto &[shares, i_asset] = compo[i];
    dx = std::max<int>(-shares + 1, dx);
    dx = std::min<int>(dx, trucs.nb_shares[i_asset] - shares);

    auto sharpe = recompute_sharpe(cache, i, dx, false);
    // Set best sharpe if better sharpe and still valid
    if (sharpe > best_sharpe && check_compo_cache(cache)) {
      best_sharpe = sharpe;
    } else {
      // Undo action
      recompute_sharpe(cache, i, -dx, true);
    }
  }

  std::cerr << "Program sharpe after opti: " << best_sharpe << '\n';
  return compo;
}

compo_t find_best_compo_stochastic(const TrucsInteressants &trucs) {
  // TODO
  return compo_t();
}