#pragma once

#include <nlohmann/json.hpp>
#include <optional>
#include <unordered_map>
#include <variant>

using json = nlohmann::json;

namespace JumpTypes {
// The API returns dictionaries of the form: { "type": "...", value: "..." }
// Instead of returning a value with the correct type.
// And since the JSON lib does not seems to be able to generate the boilerplate
// on templated structs, we create as many `jump_values` as we want types of it
#define JUMP_VALUE(N, T)                                                       \
  struct jump_value_##N {                                                      \
    T value;                                                                   \
  };                                                                           \
  NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(jump_value_##N, value)

JUMP_VALUE(unsigned, unsigned)
JUMP_VALUE(string, std::string)

struct asset {
  /** Identifiant en base de l'actif */
  jump_value_string ASSET_DATABASE_ID;

  /** Nom de l'actif */
  jump_value_string LABEL;

  /** Dernière valeur de clôture */
  // jump_value_string LAST_CLOSE_VALUE_IN_CURR;

  /** Currency */
  jump_value_string CURRENCY;

  /** Type d'actifs */
  jump_value_string TYPE;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(asset, ASSET_DATABASE_ID, LABEL, CURRENCY,
                                   TYPE)

struct ratio {
  unsigned id;
  bool is_benchmark_needed;
  bool is_percent;
  std::string name;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ratio, id, is_benchmark_needed, is_percent,
                                   name)

struct quote {
  float close;
  unsigned coupon;
  std::string date;
  float gross;
  float high;
  float low;
  float nav;
  float open;
  double pl;
  // Since C++ does not support using reserved keywords as identifier
  // and JSON lib does not support field alias,
  // we must define the conversion functions ourselves
  double v_return;
  unsigned volume;
};

inline void to_json(json &j, const quote &v) {
  j = json{{"close", v.close},     {"coupon", v.coupon}, {"date", v.date},
           {"gross", v.gross},     {"high", v.high},     {"low", v.low},
           {"nav", v.nav},         {"open", v.open},     {"pl", v.pl},
           {"return", v.v_return}, {"volume", v.volume}};
}

inline void from_json(const json &j, quote &v) {
  j.at("close").get_to(v.close);
  j.at("coupon").get_to(v.coupon);
  j.at("date").get_to(v.date);
  j.at("gross").get_to(v.gross);
  j.at("high").get_to(v.high);
  j.at("low").get_to(v.low);
  j.at("nav").get_to(v.nav);
  j.at("open").get_to(v.open);
  j.at("pl").get_to(v.pl);
  j.at("return").get_to(v.v_return);
  j.at("volume").get_to(v.volume);
}

struct currency {
  std::string code;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(currency, code)

struct portfolio_asset {
  int32_t asset;
  double quantity;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(portfolio_asset, asset, quantity)

struct portfolio_currency {
  std::string currency;
  double amount;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(portfolio_currency, currency, amount)

struct portfolio_value {
  std::optional<portfolio_asset> asset;
  std::optional<portfolio_currency> currency;
};

inline void to_json(json &j, const portfolio_value &v) {
  j = json{};
  if (v.asset.has_value()) {
    json j_asset = *v.asset;
    j["asset"] = j_asset;
  }
  if (v.currency.has_value()) {
    json j_curr = *v.currency;
    j["currency"] = j_curr;
  }
}

inline void from_json(const json &j, portfolio_value &v) {
  auto it_asset = j.find("asset");
  if (it_asset != j.end()) {
    v.asset = it_asset->get<portfolio_asset>();
  }
  auto it_curr = j.find("currency");
  if (it_curr != j.end()) {
    v.currency = it_curr->get<portfolio_currency>();
  }
}

struct portfolio {
  JumpTypes::currency currency;
  std::string label;
  std::string type;
  std::unordered_map<std::string, std::vector<portfolio_value>> values;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(portfolio, currency, label, type, values)
} // namespace JumpTypes
