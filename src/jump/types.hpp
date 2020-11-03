#pragma once

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace JumpTypes {
struct asset {
  std::string ASSET_DATABASE_ID;
  std::string CREATION_DATE;
  std::string CURRENCY;
  std::string FIRST_QUOTE_DATE;
  std::string ISIN_CODE;
  std::string LABEL;
  std::string MODIFICATION_DATE;
  std::string SUB_TYPE;
  std::string TYPE;
};

// TODO, cannot use directly
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(asset, ASSET_DATABASE_ID, CREATION_DATE,
                                   CURRENCY, FIRST_QUOTE_DATE, ISIN_CODE, LABEL,
                                   MODIFICATION_DATE, SUB_TYPE, TYPE)

struct ratio {
  unsigned id;
  bool is_benchmark_needed;
  bool is_percent;
  std::string name;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ratio, id, is_benchmark_needed, is_percent,
                                   name)
} // namespace JumpTypes
