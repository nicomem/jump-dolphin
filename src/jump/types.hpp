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
struct JumpValueString {
  std::string value;
};

enum class AssetType {
  BOND,
  FUND,
  PORTFOLIO,
  STOCK,
};

struct Asset {
  /** Identifiant en base de l'actif */
  std::string id;

  /** Nom de l'actif */
  AssetType label;

  /** Currency */
  std::string currency;

  /** Type d'actifs */
  std::string type;

  /** Dernière valeur de clôture */
  std::optional<std::string> last_close_value;
};

struct Ratio {
  unsigned id;
  bool is_benchmark_needed;
  bool is_percent;
  std::string name;
};

struct Quote {
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

struct PortfolioAsset {
  int32_t asset;
  double quantity;
};

struct PortfolioCurrency {
  std::string currency;
  double amount;
};

struct portfolio_value {
  std::optional<PortfolioAsset> asset;
  std::optional<PortfolioCurrency> currency;
};

enum class CurrencyCode { EUR, GBP, JPY, NOK, SEK, USD };

enum class DynAmountType { back, front };

struct Portfolio {
  /** Nom du portefeuille */
  std::string label;

  /** Devise. L'identifiant est le code ISO 4217 de la devise */
  CurrencyCode currency;

  /** Type de portfolio DynAmount */
  DynAmountType type;

  /** Contenu du portefeuille par date. Les clés de cet objet sont au format
   * 'date' */
  std::unordered_map<std::string, std::vector<portfolio_value>> values;
};

struct RatioParam {
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
 * <ratio_obj>: { "id_ratio": <JumpValueString> }
 */
struct AssetRatioMap {
  using ratio_obj = std::unordered_map<std::string, JumpValueString>;

  std::unordered_map<std::string, ratio_obj> value;
};

} // namespace JumpTypes
