#pragma once

#include "jump/client.hpp"
#include "tree.hpp"

struct SharpeCache {
  const TrucsInteressants &trucs;
  compo_t &compo;

  double start_capital;
  double end_capital;

  finmath::asset_period_values_t buy_values;

  SharpeCache(const TrucsInteressants &trucs_, compo_t &compo_)
      : trucs(trucs_), compo(compo_), start_capital(), end_capital(),
        buy_values() {}
};

/** Compute the sharpe of the composition and initialize the cache
 * that can after be used in `recompute_sharpe`
 */
sharpe_t compute_sharpe_init_chache(const TrucsInteressants &trucs,
                                    const compo_t &compo, SharpeCache &cache);

/** Recompute the sharpe after only one asset shares changed */
sharpe_t recompute_sharpe(SharpeCache &cache, unsigned i_compo_changed,
                          double dshares, bool only_update_cache);

bool check_compo_cache(const SharpeCache &cache);

/** Try to optimize a composition by changing the number of shares.
 * If the given composition respects the %NAV rule, the resulting
 * composition will also respect it.
 */
std::tuple<compo_t, sharpe_t>
optimize_compo_stochastic(const TrucsInteressants &trucs, compo_t compo);

/** Try to find the best composition by using the stochastic optimizer */
compo_t
find_best_compo_stochastic(const TrucsInteressants &trucs, compo_t compo,
                           std::function<double(const compo_t)> get_sharpe);
