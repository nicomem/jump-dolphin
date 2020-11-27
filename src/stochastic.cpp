#include "stochastic.hpp"
#include "check.hpp"

#include <csignal>
#include <iostream>
#include <random>

namespace {
volatile bool abort_process = false;
}

void signal_handler(int) { abort_process = true; }

static double comp_vol(const SharpeCache &cache) {
  double vol = 0;
  for (auto i = 0u; i < cache.compo.size(); ++i) {
    auto asset1 = std::get<1>(cache.compo[i]);
    const auto &cov_vec = cache.trucs.cov_matrix[asset1];

    double subvol = cache.buy_values[i] * cov_vec[asset1];
    for (auto j = i + 1; j < cache.compo.size(); ++j) {
      auto asset2 = std::get<1>(cache.compo[j]);

      subvol += 2 * cache.buy_values[j] * cov_vec[asset2];
    }
    vol += (cache.buy_values[i] / cache.start_capital) * subvol;
  }

  return vol / cache.start_capital;
}

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

  double vol = comp_vol(cache);

  // Compute the sharpe
  return (cache.end_capital / cache.start_capital) / vol;
}

sharpe_t recompute_sharpe(SharpeCache &cache, unsigned i_compo_changed,
                          double dshares, bool only_update_cache) {
  auto &[shares, i_asset] = cache.compo[i_compo_changed];
  shares += (unsigned)dshares;

  // Initialize the vectors
  auto compo_size = cache.compo.size();
  cache.buy_values.resize(0);
  cache.buy_values.reserve(compo_size);

  // Set the start values & capital
  cache.start_capital = 0;
  for (const auto &[nb_shares, i_asset] : cache.compo) {
    cache.start_capital += cache.buy_values.emplace_back(
        (double)nb_shares * cache.trucs.start_values[i_asset]);
  }

  // Set the end values & capital
  cache.end_capital = 0;
  for (const auto &[nb_shares, i_asset] : cache.compo) {
    cache.end_capital += (double)nb_shares * cache.trucs.end_values[i_asset];
  }

  // Update start & end capital
  auto dbuy_value = (double)dshares * cache.trucs.start_values[i_asset];
  cache.buy_values[i_compo_changed] += dbuy_value;
  cache.start_capital += dbuy_value;

  cache.end_capital += dshares * cache.trucs.end_values[i_asset];

  if (only_update_cache || !check_compo_cache(cache))
    return -INFINITY;

  double vol = comp_vol(cache);

  // Compute the sharpe
  return (cache.end_capital / cache.start_capital) / vol;
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
optimize_compo_stochastic(const TrucsInteressants &trucs, compo_t compo) {
  constexpr auto n_iter = 5000u;

  std::random_device rd;
  std::mt19937 gen(rd());

  auto cache = SharpeCache(trucs, compo);
  auto best_sharpe = compute_sharpe_init_chache(trucs, compo, cache);
  // best_sharpe = get_sharpe(compo);

  auto dshare = std::normal_distribution<double>(0, 1);
  auto dasset = std::uniform_int_distribution<unsigned>(0, compo.size() - 1);
  auto best_compo = compo;
  for (auto _i = 0u; _i < n_iter; ++_i) {
    int dx;
    do {
      dx = 50 * dshare(gen);
    } while (dx == 0);

    share_index_t i = dasset(gen);

    // Clamp the share modifier to be in bound
    auto &[shares, i_asset] = compo[i];
    dx = std::max<int>(-shares + 1, dx);
    dx = std::min<int>(dx, trucs.nb_shares[i_asset] - shares);

    auto sharpe_opt = recompute_sharpe(cache, i, dx, false);
    // std::cout << sharpe_opt << " --- " << best_sharpe << '\n';
    // Set best sharpe if better sharpe and still valid
    if (sharpe_opt > 0 && sharpe_opt > best_sharpe) {
      best_sharpe = sharpe_opt;
      best_compo = compo;
    } else {
      // Undo action
      recompute_sharpe(cache, i, -dx, true);
      compo = best_compo;
    }
  }

  auto go_one_way = [&cache, &best_sharpe](auto i, auto step,
                                           auto &found_better) -> bool {
    auto &[shares, i_asset] = cache.compo[i];
    // shares += step;

    auto out_of_bounds =
        shares + step < 1 || shares + step >= cache.trucs.nb_shares[i_asset];
    if (out_of_bounds || check_compo_cache(cache)) {
      shares -= step;
      return false;
    }

    auto sharpe = recompute_sharpe(cache, i, step, false);
    if (sharpe > best_sharpe) {
      best_sharpe = sharpe;
      found_better = true;
      return true;
    } else {
      shares -= step;
      return false;
    }
  };

  best_sharpe = compute_sharpe_init_chache(trucs, best_compo, cache);

  // Optimize with a big step
  constexpr int step1 = 10;
  bool found_better;
  do {
    found_better = false;

    for (auto i = 0u; i < compo.size(); ++i) {
      // Test one way
      while (go_one_way(i, step1, found_better)) {
      }

      // Test the other way
      while (go_one_way(i, -step1, found_better)) {
      }
    }
  } while (found_better);

  return std::make_tuple(best_compo, best_sharpe);
}

std::tuple<compo_t, sharpe_t>
optimize_compo_2(const TrucsInteressants &trucs, compo_t compo, sharpe_t sharpe,
                 std::function<double(const compo_t &)> get_sharpe, bool quick,
                 bool verbose) {
  auto best_sharpe = sharpe;

  auto go_one_way = [&compo, &trucs, &get_sharpe, &best_sharpe](
                        auto i, auto step, auto &found_better) -> bool {
    auto &[shares, i_asset] = compo[i];
    shares += step;

    auto out_of_bounds = shares < 1 || shares >= trucs.nb_shares[i_asset];
    if (out_of_bounds || !check_compo(trucs, compo, false)) {
      shares -= step;
      return false;
    }

    auto sharpe = get_sharpe(compo);
    if (sharpe > best_sharpe) {
      best_sharpe = sharpe;
      found_better = true;
      return true;
    } else {
      shares -= step;
      return false;
    }
  };

  auto opti_step = [&best_sharpe, &compo, &go_one_way,
                    verbose](std::string_view name,
                             std::function<unsigned(unsigned)> get_step) {
    bool found_better;
    do {
      found_better = false;
      if (verbose) {
        std::cout << "Best sharpe (" << name << "): " << best_sharpe << '\n';
      }

      for (auto i = 0u; i < compo.size(); ++i) {
        auto step = get_step(i);

        bool found_better1 = false;
        // Test one way
        while (go_one_way(i, step, found_better1)) {
        }

        if (!found_better) {
          // Test the other way
          while (go_one_way(i, -step, found_better1)) {
          }
        }

        found_better |= found_better1;
      }
    } while (found_better);
  };

  // Optimize with a big dynamic step
  opti_step("ratio=0.1", [&trucs, &compo](auto i) -> unsigned {
    return std::max<unsigned>(1, 0.5 * min_share_percent *
                                     trucs.nb_shares[std::get<1>(compo[i])]);
  });

  if (quick) {
    return std::make_tuple(compo, best_sharpe);
  }

  // Optimize with a big fixed step
  opti_step("step=10", [](auto) -> unsigned { return 10; });

  // Optimize with a small fixed step
  opti_step("step=1", [](auto) -> unsigned { return 1; });

  return std::make_tuple(compo, best_sharpe);
}

static void swap_low_capital_ratio(const TrucsInteressants &trucs,
                                   compo_t &compo, std::mt19937 &gen,
                                   std::vector<bool> &assets_selected,
                                   bool swap_all = false) {
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

  auto cap_limit =
      std::uniform_real_distribution<double>(min_capital, max_capital)(gen);
  auto asset_gen = std::uniform_int_distribution<unsigned>(0, compo.size() - 1);

  // Swap those with low capital ratios
  for (auto &[shares, i_asset] : compo) {
    auto capital = ((double)shares * trucs.start_values[i_asset]);
    if (!swap_all && capital > cap_limit)
      continue;

    // Select random new asset
    unsigned new_asset = asset_gen(gen);
    while (assets_selected[new_asset]) {
      new_asset = (new_asset + 1) % assets_selected.size();
    }

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
                           std::function<double(const compo_t &)> get_sharpe) {
  std::signal(SIGINT, signal_handler);
  std::signal(SIGABRT, signal_handler);

  std::random_device rd;
  std::mt19937 gen(rd());
  auto assets_selected = std::vector<bool>();

  auto WANTED_PORTFOLIO_SIZE = 20u;
  WANTED_PORTFOLIO_SIZE =
      std::min<unsigned>(trucs.assets_id.size() - 1, WANTED_PORTFOLIO_SIZE);
  compo.resize(WANTED_PORTFOLIO_SIZE);

  swap_low_capital_ratio(trucs, compo, gen, assets_selected, true);

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

  constexpr auto min_sharpe_can_opti = 2.0;

  auto [best_compo, best_sharpe] = optimize_compo_stochastic(trucs, compo);
  best_sharpe = get_sharpe(compo);
  if (best_sharpe > min_sharpe_can_opti) {
    std::tie(best_compo, best_sharpe) =
        optimize_compo_2(trucs, best_compo, best_sharpe, get_sharpe, true);
  }

  std::clog << "Start compute sharpe: " << best_sharpe << '\n';
  while (!abort_process) {
    // Swap those with low capital ratios with random ones
    compo = best_compo;
    swap_low_capital_ratio(trucs, compo, gen, assets_selected);

    auto [new_compo, new_sharpe] = optimize_compo_stochastic(trucs, compo);
    new_sharpe = get_sharpe(new_compo);
    if (new_sharpe > min_sharpe_can_opti) {
      std::tie(new_compo, new_sharpe) =
          optimize_compo_2(trucs, new_compo, new_sharpe, get_sharpe, true);
    }

    std::clog << new_sharpe << ' ' << best_sharpe << '\n';
    if (new_sharpe > best_sharpe) {
      std::clog << "New best compute sharpe: " << new_sharpe << '\n';
      check_compo(trucs, best_compo, true);
      best_compo = new_compo;
      best_sharpe = new_sharpe;
    }
  }

  std::clog << "Final compute sharpe: " << best_sharpe << '\n';
  return best_compo;
}

std::tuple<compo_t, sharpe_t>
optimize_compo_3(const TrucsInteressants &trucs, compo_t compo, sharpe_t sharpe,
                 std::function<double(const compo_t &)> get_sharpe) {
  bool found_better;
  auto nb_assets = trucs.nb_shares.size();
  auto assets_selected = std::vector<bool>(nb_assets);
  for (auto &[_, i_asset] : compo) {
    assets_selected[i_asset] = true;
  }

  auto best_compo = compo;
  auto best_sharpe = sharpe;
  std::cout << "Starting sharpe: " << best_sharpe << '\n';
  do {
    found_better = false;
    for (auto i = 0u; i < compo.size(); ++i) {
      std::clog << "Optimizing compo[" << i << "]\n";
      for (auto j_asset = 0u; j_asset < nb_assets; ++j_asset) {
        if (assets_selected[j_asset])
          continue;

        std::clog << "Checking asset " << j_asset << "\n";

        auto &[nb_shares, i_asset] = compo[i];

        // Swap
        assets_selected[i_asset] = false;
        assets_selected[j_asset] = true;
        i_asset = j_asset;

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

        // Optimize and get sharpe
        auto sharpe = get_sharpe(compo);
        auto [new_compo, new_sharpe] =
            optimize_compo_2(trucs, compo, sharpe, get_sharpe, false, false);

        // Update if better
        if (new_sharpe > best_sharpe) {
          std::cout << "Found better compo with sharpe: " << new_sharpe << '\n';
          check_compo(trucs, new_compo, true);
          best_sharpe = new_sharpe;
          best_compo = new_compo;
          found_better = true;
        } else {
          compo = best_compo;
        }
      }
    }
  } while (found_better);

  return std::make_tuple(best_compo, best_sharpe);
}
