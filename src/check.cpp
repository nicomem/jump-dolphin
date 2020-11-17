#include "check.hpp"

#include <iomanip>
#include <iostream>

bool check_compo(const TrucsInteressants &trucs, const compo_t &compo,
                 bool verbose) {
  // Verify the %NAV
  bool found_error = false;

  auto compo_cap = portolio_capital(trucs, compo);
  for (const auto &[nb_shares, i_asset] : compo) {
    // Verify the portfolio conditions
    auto share_capital = nb_shares * trucs.start_values[i_asset];
    auto true_ratio = share_capital / compo_cap;

    std::cout << "- " << trucs.assets_id[i_asset] << ": "
              << std::setprecision(5) << 100 * true_ratio << "%\n";

    if (true_ratio < min_share_percent || true_ratio > max_share_percent) {
      found_error = true;

      if (verbose) {
        std::cerr << "ERROR: Asset " << trucs.assets_id[i_asset]
                  << " ratio: " << true_ratio << '\n';
        std::cerr << "--> Bought " << nb_shares << " * "
                  << trucs.start_values[i_asset] << "€ = " << share_capital
                  << '\n';
      } else {
        return false;
      }
    }
  }

  // TODO: Check stock proportion

  return !found_error;
}
