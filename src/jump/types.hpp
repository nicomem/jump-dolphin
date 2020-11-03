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

// TODO, cannot use directly
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
} // namespace JumpTypes
