#pragma once

#include "jump/client.hpp"
#include "jump/types_json_light.hpp"

#include <date/date.h>

struct SaveData {
  using DateStr = std::string;
  using DaysAssets =
      std::unordered_map<DateStr, std::vector<CompactTypes::Asset>>;

  /** Get every asset of the investment period */
  static DaysAssets every_days_assets(JumpClient &client, bool verbose = false);
};
