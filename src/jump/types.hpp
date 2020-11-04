#pragma once

#include <nlohmann/json.hpp>
#include <optional>
#include <unordered_map>

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
  jump_value_string id;

  /** Nom de l'actif */
  jump_value_string label;

  /** Currency */
  jump_value_string currency;

  /** Type d'actifs */
  jump_value_string type;

  /** Dernière valeur de clôture */
  std::optional<jump_value_string> last_close_value;
};

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
  TO_JSON_OPT(j, v, "asset", asset);
  TO_JSON_OPT(j, v, "currency", currency);
}

inline void from_json(const json &j, portfolio_value &v) {
  FROM_JSON_OPT(j, v, "asset", asset);
  FROM_JSON_OPT(j, v, "currency", currency);
}

enum class dyn_amount_type { back, front };

NLOHMANN_JSON_SERIALIZE_ENUM(dyn_amount_type,
                             {{dyn_amount_type::back, "back"},
                              {dyn_amount_type::front, "front"}})

struct portfolio {
  /** Nom du portefeuille */
  std::string label;

  /** Devise. L'identifiant est le code ISO 4217 de la devise */
  JumpTypes::currency currency;

  /** Type de portfolio DynAmount */
  dyn_amount_type type;

  /** Contenu du portefeuille par date. Les clés de cet objet sont au format
   * 'date' */
  std::unordered_map<std::string, std::vector<portfolio_value>> values;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(portfolio, currency, label, type, values)

struct ratio_param {
  /** Id des ratios à éxécuter */
  std::vector<int32_t> ratio;

  /** Id des actifs sur lesquels éxécuter le ratio */
  std::vector<int32_t> asset;

  /** Benchmark pour le ratio, Peut être nécessaire selon les ratios, voir la
   * propriété du ratio 'is_benchmark_needed' */
  std::optional<int32_t> benchmark;

  /** Date de debut pour le ratio, Peut être nécessaire selon les ratios
   * end_date */
  std::optional<std::string> date;

  /** Date de fin pour le ratio, Peut être nécessaire selon les ratios */
  std::optional<std::string> end_date;
};

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
} // namespace JumpTypes
