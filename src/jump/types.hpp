#pragma once

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace JumpTypes {
struct JumpValue {
  std::string value;
  std::string type;
};

enum class AssetLabel {
  BOND,
  FUND,
  PORTFOLIO,
  STOCK,
};

enum class AssetType {
  ETF_FUND,
  FUND,
  INDEX,
  PORTFOLIO,
  STOCK,
};

enum class CurrencyCode { EUR, GBP, JPY, NOK, SEK, USD };

inline unsigned index(CurrencyCode value) {
  switch (value) {
  case CurrencyCode::EUR:
    return 0;

    // case CurrencyCode::GBP:
    //  return 1;

  case CurrencyCode::JPY:
    return 1;

    // case CurrencyCode::NOK:
    //  return 3;

    // case CurrencyCode::SEK:
    //  return 4;

  case CurrencyCode::USD:
    return 2;

  default:
    return 0;
  }
}

static constexpr auto currencies = std::array{
    CurrencyCode::EUR,
    // CurrencyCode::GBP,
    CurrencyCode::JPY,
    // CurrencyCode::NOK,
    // CurrencyCode::SEK,
    CurrencyCode::USD,
};

struct AssetValue {
  /** Value of a share of the asset */
  double value;

  /** Currency of the asset share */
  CurrencyCode currency;
};

struct Asset {
  /** Identifiant en base de l'actif */
  std::string id;

  /** Nom de l'actif */
  AssetLabel label;

  /** Currency */
  CurrencyCode currency;

  /** Type d'actifs */
  AssetType type;

  /** Dernière valeur de clôture */
  std::optional<AssetValue> last_close_value;
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

static constexpr std::string_view currency_str(CurrencyCode code) {
  switch (code) {
  case CurrencyCode::EUR:
    return "EUR";
  case CurrencyCode::GBP:
    return "GBP";
  case CurrencyCode::JPY:
    return "JPY";
  case CurrencyCode::NOK:
    return "NOK";
  case CurrencyCode::SEK:
    return "SEK";
  case CurrencyCode::USD:
    return "USD";
  default:
    return "";
  }
}

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
  RatioParam() = default;
  RatioParam(RatioParam&&) = default;

  /** Id des ratios à éxécuter */
  std::vector<int32_t> ratio;

  /** Id des actifs sur lesquels éxécuter le ratio */
  std::vector<int32_t> asset;

  /** Benchmark pour le ratio, Peut être nécessaire selon les ratios, voir la
   * propriété du ratio 'is_benchmark_needed' */
  std::optional<int32_t> benchmark;

  /** Date de debut pour le ratio, Peut être nécessaire selon les ratios
   * end_date */
  std::optional<std::string> start_date;

  /** Date de fin pour le ratio, Peut être nécessaire selon les ratios */
  std::optional<std::string> end_date;
};

/** Un objet référençant chacun des actifs, eux-mêmes étant des objets
 * référençant chaque ratio exécuté avec succès contenant la valeur calculée:
 *
 * <asset_ratio_map>: { "id_actif": <ratio_obj> }
 * <ratio_obj>: { "id_ratio": <JumpValue> }
 */
struct AssetRatioMap {
  using ratio_obj = std::unordered_map<std::string, JumpValue>;

  std::unordered_map<std::string, ratio_obj> value;
};

} // namespace JumpTypes
