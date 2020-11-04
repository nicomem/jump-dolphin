#pragma once

#include "types.hpp"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace JumpTypes {
// Define helpers to (de)serialize optional fields
#define TO_JSON(J, V, ID, FIELD)                                               \
  do {                                                                         \
    j[ID] = V.FIELD;                                                           \
  } while (0)

#define FROM_JSON(J, V, ID, FIELD)                                             \
  do {                                                                         \
    V.FIELD = J.at(ID).get<decltype(V.FIELD)>();                               \
  } while (0)

#define TO_JSON_OPT(J, V, ID, FIELD)                                           \
  do {                                                                         \
    if (V.FIELD.has_value()) {                                                 \
      json j_v = *V.FIELD;                                                     \
      J[ID] = j_v;                                                             \
    }                                                                          \
  } while (0)

#define FROM_JSON_OPT(J, V, ID, FIELD)                                         \
  do {                                                                         \
    auto it = J.find(ID);                                                      \
    if (it != J.end()) {                                                       \
      V.FIELD = it->get<decltype(V.FIELD)::value_type>();                      \
    }                                                                          \
  } while (0)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(jump_value_string, value)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(jump_value_unsigned, value)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ratio, id, is_benchmark_needed, is_percent,
                                   name)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(currency, code)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(portfolio_asset, asset, quantity)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(portfolio_currency, currency, amount)

inline void to_json(json &j, const portfolio_value &v) {
  j = json{};
  TO_JSON_OPT(j, v, "asset", asset);
  TO_JSON_OPT(j, v, "currency", currency);
}

inline void from_json(const json &j, portfolio_value &v) {
  FROM_JSON_OPT(j, v, "asset", asset);
  FROM_JSON_OPT(j, v, "currency", currency);
}

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(portfolio, currency, label, type, values)

NLOHMANN_JSON_SERIALIZE_ENUM(dyn_amount_type,
                             {{dyn_amount_type::back, "back"},
                              {dyn_amount_type::front, "front"}})

inline void to_json(json &j, const asset &v) {
  j = json{};
  TO_JSON(j, v, "ASSET_DATABASE_ID", id);
  TO_JSON(j, v, "LABEL", label);
  TO_JSON(j, v, "CURRENCY", currency);
  TO_JSON(j, v, "TYPE", type);
  TO_JSON_OPT(j, v, "LAST_CLOSE_VALUE_IN_CURR", last_close_value);
}

inline void from_json(const json &j, asset &v) {
  FROM_JSON(j, v, "ASSET_DATABASE_ID", id);
  FROM_JSON(j, v, "LABEL", label);
  FROM_JSON(j, v, "CURRENCY", currency);
  FROM_JSON(j, v, "TYPE", type);
  FROM_JSON_OPT(j, v, "LAST_CLOSE_VALUE_IN_CURR", last_close_value);
}

inline void to_json(json &j, const quote &v) {
  j = json{};
  TO_JSON(j, v, "close", close);
  TO_JSON(j, v, "coupon", coupon);
  TO_JSON(j, v, "date", date);
  TO_JSON(j, v, "gross", gross);
  TO_JSON(j, v, "high", high);
  TO_JSON(j, v, "low", low);
  TO_JSON(j, v, "nav", nav);
  TO_JSON(j, v, "open", open);
  TO_JSON(j, v, "return", v_return);
  TO_JSON(j, v, "volume", volume);
}

inline void from_json(const json &j, quote &v) {
  FROM_JSON(j, v, "close", close);
  FROM_JSON(j, v, "coupon", coupon);
  FROM_JSON(j, v, "date", date);
  FROM_JSON(j, v, "gross", gross);
  FROM_JSON(j, v, "high", high);
  FROM_JSON(j, v, "low", low);
  FROM_JSON(j, v, "nav", nav);
  FROM_JSON(j, v, "open", open);
  FROM_JSON(j, v, "return", v_return);
  FROM_JSON(j, v, "volume", volume);
}

inline void to_json(json &j, const ratio_param &v) {
  j = json{};
  TO_JSON(j, v, "ratio", ratio);
  TO_JSON(j, v, "asset", asset);
  TO_JSON_OPT(j, v, "benchmark", benchmark);
  TO_JSON_OPT(j, v, "date", date);
  TO_JSON_OPT(j, v, "end_date", end_date);
}

inline void from_json(const json &j, ratio_param &v) {
  FROM_JSON(j, v, "ratio", ratio);
  FROM_JSON(j, v, "asset", asset);
  FROM_JSON_OPT(j, v, "benchmark", benchmark);
  FROM_JSON_OPT(j, v, "date", date);
  FROM_JSON_OPT(j, v, "end_date", end_date);
}

inline void to_json(json &j, const asset_ratio_map &v) { j = v.value; }

inline void from_json(const json &j, asset_ratio_map &v) {
  v.value = j.get<decltype(v.value)>();
}
} // namespace JumpTypes
