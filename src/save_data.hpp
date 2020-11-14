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
                      JumpClient &client, bool verbose = false);

  /** Extract the assets values for the last investment day.
   * Store/Use the DaysAssets in the given parameter if the save file is not
   * present.
   */
  static finmath::assets_day_values_t
  assets_end_values(std::optional<DaysAssets> &days_assets, JumpClient &client,
                    bool verbose = false);

  /** Compute the covariance matrix between each couple of assets
   * Store/Use the DaysAssets in the given parameter if the save file is not
   * present.
   */
  static finmath::covariance_matrix_t
  covariance_matrix(std::optional<DaysAssets> &days_assets, JumpClient &client,
                    bool verbose = false);
};
