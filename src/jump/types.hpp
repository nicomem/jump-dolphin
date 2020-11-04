#pragma once

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace JumpTypes {
// The API returns dictionaries of the form: { "type": "...", value: "..." }
// Instead of returning a value with the correct type.
// And since the JSON lib does not seems to be able to generate the boilerplate
// on templated structs, we create as many `jump_values` as we want types of it
struct jump_value_unsigned {
  unsigned value;
};

struct jump_value_string {
  std::string value;
};

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

struct ratio {
  unsigned id;
  bool is_benchmark_needed;
  bool is_percent;
  std::string name;
};

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

struct currency {
  std::string code;
};

struct portfolio_asset {
  int32_t asset;
  double quantity;
};

struct portfolio_currency {
  std::string currency;
  double amount;
};

struct portfolio_value {
  std::optional<portfolio_asset> asset;
  std::optional<portfolio_currency> currency;
};

enum class dyn_amount_type { back, front };

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

/** Un objet référençant chacun des actifs, eux-mêmes étant des objets
 * référençant chaque ratio exécuté avec succès contenant la valeur calculée:
 *
 * <asset_ratio_map>: { "id_actif": <ratio_obj> }
 * <ratio_obj>: { "id_ratio": <jump_value_string> }
 */
struct asset_ratio_map {
  using ratio_obj = std::unordered_map<std::string, jump_value_string>;

  std::unordered_map<std::string, ratio_obj> value;
};

} // namespace JumpTypes
