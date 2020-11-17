#include "optimizer.hpp"
#include "check.hpp"

#include <iostream>
#include <random>

compo_t optimize_compo_stochastic(const TrucsInteressants &trucs,
                                  compo_t compo) {
  constexpr auto n_iter = 1'000'00u;

  std::random_device rd;
  std::mt19937 gen(rd());

  auto shares_capital = std::vector<double>();
  shares_capital.reserve(compo.size());

  auto best_sharpe = compute_sharpe(trucs, compo, shares_capital);
  std::cerr << "Sharpe before opti: " << best_sharpe << '\n';

  auto dshare = std::normal_distribution<double>(0, 1);
  auto dasset = std::uniform_int_distribution<unsigned>(0, compo.size() - 1);
  for (auto _i = 0u; _i < n_iter; ++_i) {
    share_index_t i = dasset(gen);
    int dx = dshare(gen);

    auto &[shares, i_asset] = compo[i];
    auto old = shares;

    shares =
        std::max(1, std::min((int)shares + dx, (int)trucs.nb_shares[i_asset]));

    auto sharpe = compute_sharpe(trucs, compo, shares_capital);
    // Set best sharpe if better sharpe and still valid
    if (sharpe > best_sharpe && check_compo(trucs, compo, false)) {
      best_sharpe = sharpe;
    } else {
      // Undo action
      shares = old;
    }
  }

  std::cerr << "Sharpe after opti: " << best_sharpe << '\n';

  return compo;
}
