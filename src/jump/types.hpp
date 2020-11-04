#pragma once

#include <nlohmann/json.hpp>

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
} // namespace JumpTypes
