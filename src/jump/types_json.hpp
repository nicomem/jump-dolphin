#pragma once

#include "types.hpp"

#include <sstream>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace JumpTypes {
// Define helpers to (de)serialize
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

// Helpers for jump_values, to avoid the inner struct
#define TO_JSON_JVAL(J, V, ID, FIELD)                                          \
  do {                                                                         \
    j[ID]["value"] = V.FIELD;                                                  \
  } while (0)

#define FROM_JSON_JVAL(J, V, ID, FIELD)                                        \
  do {                                                                         \
    V.FIELD = J.at(ID).at("value").get<decltype(V.FIELD)>();                   \
  } while (0)

#define TO_JSON_OPT_JVAL(J, V, ID, FIELD)                                      \
  do {                                                                         \
    if (V.FIELD.has_value()) {                                                 \
      json j_v = *V.FIELD;                                                     \
      J[ID]["value"] = j_v;                                                    \
    }                                                                          \
  } while (0)

#define FROM_JSON_OPT_JVAL(J, V, ID, FIELD)                                    \
  do {                                                                         \
    auto it = J.find(ID);                                                      \
    if (it != J.end()) {                                                       \
      V.FIELD = it->at("value").get<decltype(V.FIELD)::value_type>();          \
    }                                                                          \
  } while (0)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(JumpValue, value, type)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Ratio, id, is_benchmark_needed, is_percent,
                                   name)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PortfolioAsset, asset, quantity)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PortfolioCurrency, currency, amount)

NLOHMANN_JSON_SERIALIZE_ENUM(AssetLabel,
                             {
                                 {AssetLabel::BOND, "BOND"},
                                 {AssetLabel::FUND, "FUND"},
                                 {AssetLabel::PORTFOLIO, "PORTFOLIO"},
                                 {AssetLabel::STOCK, "STOCK"},
                             })

NLOHMANN_JSON_SERIALIZE_ENUM(AssetType, {
                                            {AssetType::ETF_FUND, "ETF FUND"},
                                            {AssetType::FUND, "FUND"},
                                            {AssetType::INDEX, "INDEX"},
                                            {AssetType::PORTFOLIO, "PORTFOLIO"},
                                            {AssetType::STOCK, "STOCK"},
                                        })

NLOHMANN_JSON_SERIALIZE_ENUM(
    CurrencyCode,
    {
        {CurrencyCode::EUR, "EUR"},
        {CurrencyCode::GBP, "GBp"}, // No that is not a mistake (from our part)
        {CurrencyCode::JPY, "JPY"},
        {CurrencyCode::NOK, "NOK"},
        {CurrencyCode::SEK, "SEK"},
        {CurrencyCode::USD, "USD"},
    })

NLOHMANN_JSON_SERIALIZE_ENUM(DynAmountType, {{DynAmountType::back, "back"},
                                             {DynAmountType::front, "front"}})

inline void to_json(json &j, const AssetValue &v) {
  auto ss = std::stringstream("");
  ss.imbue(std::locale("fr_FR.UTF-8"));
  ss << std::fixed << v.value << ' ';

  json j_curr = v.currency;
  ss << j_curr;

  j = json{{"type", "string"}, {"value", ss.str()}};
}

inline void from_json(const json &j, AssetValue &v) {
  auto stream = std::istringstream(j.get<std::string>());

  std::string token;
  std::getline(stream, token, ' ');

  std::replace(token.begin(), token.end(), ',', '.');
  v.value = std::stod(token);

  json j_curr = stream.str();
  from_json(j_curr, v.currency);
}

inline void to_json(json &j, const Asset &v) {
  j = json{};
  TO_JSON_JVAL(j, v, "ASSET_DATABASE_ID", id);
  TO_JSON_JVAL(j, v, "LABEL", label);
  TO_JSON_JVAL(j, v, "CURRENCY", currency);
  TO_JSON_JVAL(j, v, "TYPE", type);
  TO_JSON_OPT_JVAL(j, v, "LAST_CLOSE_VALUE_IN_CURR", last_close_value);
}

inline void from_json(const json &j, Asset &v) {
  FROM_JSON_JVAL(j, v, "ASSET_DATABASE_ID", id);
  FROM_JSON_JVAL(j, v, "LABEL", label);
  FROM_JSON_JVAL(j, v, "CURRENCY", currency);
  FROM_JSON_JVAL(j, v, "TYPE", type);
  FROM_JSON_OPT_JVAL(j, v, "LAST_CLOSE_VALUE_IN_CURR", last_close_value);
}

inline void to_json(json &j, const Quote &v) {
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

inline void from_json(const json &j, Quote &v) {
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

inline void to_json(json &j, const portfolio_value &v) {
  j = json{};
  TO_JSON_OPT(j, v, "asset", asset);
  TO_JSON_OPT(j, v, "currency", currency);
}

inline void from_json(const json &j, portfolio_value &v) {
  FROM_JSON_OPT(j, v, "asset", asset);
  FROM_JSON_OPT(j, v, "currency", currency);
}

inline void to_json(json &j, const Portfolio &v) {
  j = json{};
  TO_JSON(j, v, "label", label);
  TO_JSON(j.at("currency"), v, "code", currency);
  TO_JSON(j, v, "type", type);
  TO_JSON(j, v, "values", values);
}

inline void from_json(const json &j, Portfolio &v) {
  FROM_JSON(j, v, "label", label);
  FROM_JSON(j.at("currency"), v, "code", currency);
  FROM_JSON(j, v, "type", type);
  FROM_JSON(j, v, "values", values);
}

inline void to_json(json &j, const RatioParam &v) {
  j = json{};
  TO_JSON(j, v, "ratio", ratio);
  TO_JSON(j, v, "asset", asset);
  TO_JSON_OPT(j, v, "benchmark", benchmark);
  TO_JSON_OPT(j, v, "date", date);
  TO_JSON_OPT(j, v, "end_date", end_date);
}

inline void from_json(const json &j, RatioParam &v) {
  FROM_JSON(j, v, "ratio", ratio);
  FROM_JSON(j, v, "asset", asset);
  FROM_JSON_OPT(j, v, "benchmark", benchmark);
  FROM_JSON_OPT(j, v, "date", date);
  FROM_JSON_OPT(j, v, "end_date", end_date);
}

inline void to_json(json &j, const AssetRatioMap &v) { j = v.value; }

inline void from_json(const json &j, AssetRatioMap &v) {
  v.value = j.get<decltype(v.value)>();
}
} // namespace JumpTypes
