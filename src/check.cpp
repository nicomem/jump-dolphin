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

    if (verbose) {
      std::cout << "- " << trucs.assets_id[i_asset] << " (" << std::setw(4)
                << nb_shares << "): " << std::setprecision(3)
                << 100 * true_ratio << "%";
    }

    if (true_ratio < min_share_percent || true_ratio > max_share_percent) {
      found_error = true;

      if (verbose) {
        std::cout << "\t!!!ERROR!!!\n";
      } else {
        return false;
      }
    } else if (verbose) {
      std::cout << '\n';
    }
  }

  // TODO: Check stock proportion

  return !found_error;
}
