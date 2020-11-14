#pragma once

#include "finmath.hpp"
#include "jump/client.hpp"
#include "jump/types_json_light.hpp"

#include <date/date.h>

struct SaveData {
  using DateStr = std::string;
  using DaysAssets =
      std::unordered_map<DateStr, std::vector<CompactTypes::Asset>>;

  /** Get every asset of the investment period */
  static DaysAssets every_days_assets(JumpClient &client, bool verbose = false);

  /** Extract the assets values for the first investment day.
   * Store/Use the DaysAssets in the given parameter if the save file is not
   * present.
   */
  static finmath::assets_day_values_t
  assets_start_values(std::optional<DaysAssets> &days_assets,
                      const finmath::day_currency_rates_t &start_rates,
                      JumpClient &client, bool verbose = false);

  /** Extract the assets values for the last investment day.
   * Store/Use the DaysAssets in the given parameter if the save file is not
   * present.
   */
  static finmath::assets_day_values_t
  assets_end_values(std::optional<DaysAssets> &days_assets,
                    const finmath::day_currency_rates_t &end_rates,
                    JumpClient &client, bool verbose = false);

  /** Compute the covariance matrix between each couple of assets
   * Store/Use the DaysAssets in the given parameter if the save file is not
   * present.
   */
  static finmath::covariance_matrix_t
  covariance_matrix(std::optional<DaysAssets> &days_assets,
                    std::optional<finmath::days_currency_rates_t> &days_rates,
                    JumpClient &client, bool verbose = false);

  /** Get every rates for every currencies -- from currency to EUR -- for the
   * invesment period
   */
  static finmath::days_currency_rates_t
  days_currency_rates(JumpClient &client, bool verbose = false);

  /** Get the the first day currency rates
   */
  static finmath::day_currency_rates_t
  start_date_currency_rate(std::optional<finmath::days_currency_rates_t> &days,
                           JumpClient &client, bool verbose = false);

  /** Get the the last day currency rates
   */
  static finmath::day_currency_rates_t
  end_date_currency_rate(std::optional<finmath::days_currency_rates_t> &days,
                         JumpClient &client, bool verbose = false);
};
