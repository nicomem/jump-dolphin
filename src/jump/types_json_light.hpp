#include "types.hpp"

#include <sstream>

#include "types_json.hpp"

#include <fmt/format.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

#define CTORMOVE(ID) ID(std::move(other.ID))
#define CTORENUMSWITCH(ID)                                                     \
  case Other::ID:                                                              \
    value = ID;                                                                \
    break;

namespace CompactTypes {
struct CurrencyCode {
  enum _enum : unsigned char {
    EUR = 'E',
    GBP = 'G',
    JPY = 'J',
    NOK = 'N',
    SEK = 'S',
    USD = 'U',
  };

  _enum value;

  CurrencyCode() : value(EUR) {}
  CurrencyCode(unsigned char c) : value(_enum(c)) {}
  CurrencyCode(JumpTypes::CurrencyCode &&other) {
    using Other = JumpTypes::CurrencyCode;
    switch (other) {
      CTORENUMSWITCH(EUR);
      CTORENUMSWITCH(GBP);
      CTORENUMSWITCH(JPY);
      CTORENUMSWITCH(NOK);
      CTORENUMSWITCH(SEK);
      CTORENUMSWITCH(USD);
    }
  }
};

struct AssetLabel {
  enum _enum : unsigned char {
    BOND = 'B',
    FUND = 'F',
    PORTFOLIO = 'P',
    STOCK = 'S',
  };

  _enum value;

  AssetLabel() : value(BOND) {}
  AssetLabel(unsigned char c) : value(_enum(c)) {}
  AssetLabel(JumpTypes::AssetLabel &&other) {
    using Other = JumpTypes::AssetLabel;
    switch (other) {
      CTORENUMSWITCH(BOND);
      CTORENUMSWITCH(FUND);
      CTORENUMSWITCH(PORTFOLIO);
      CTORENUMSWITCH(STOCK);
    }
  }
};

struct AssetType {
  enum _enum : unsigned char {
    ETF_FUND = 'E',
    FUND = 'F',
    INDEX = 'I',
    PORTFOLIO = 'P',
    STOCK = 'S',
  };

  _enum value;

  AssetType() : value(ETF_FUND) {}
  AssetType(unsigned char c) : value(_enum(c)) {}
  AssetType(JumpTypes::AssetType &&other) {
    using Other = JumpTypes::AssetType;
    switch (other) {
      CTORENUMSWITCH(ETF_FUND);
      CTORENUMSWITCH(FUND);
      CTORENUMSWITCH(INDEX);
      CTORENUMSWITCH(PORTFOLIO);
      CTORENUMSWITCH(STOCK);
    }
  }
};

struct AssetValue {
  /** Value of a share of the asset */
  double value;

  /** Currency of the asset share */
  CurrencyCode currency;

  AssetValue() = default;
  AssetValue(JumpTypes::AssetValue &&other)
      : CTORMOVE(value), CTORMOVE(currency) {}
};

inline void to_json(json &j, const AssetValue &v) {
  auto ss = std::stringstream("");

  ss << (int)v.value;

  // Create str representation of double with a comma instead of dot
  auto v_str = fmt::format("{}", v.value);
  auto dot = v_str.find('.');
  ss << ',' << v_str.substr(dot + 1) << ' ' << v.currency.value;

  j = json::string_t(ss.str());
}

inline void from_json(const json &j, AssetValue &v) {
  auto stream = std::istringstream(j.get<std::string>());

  std::string token;
  std::getline(stream, token, ' ');

  std::replace(token.begin(), token.end(), ',', '.');
  v.value = std::stod(token);

  v.currency = CurrencyCode(stream.get());
}

/** Version of an asset that have a more compact serialization */
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

  Asset() = default;
  Asset(JumpTypes::Asset &&other)
      : CTORMOVE(id), CTORMOVE(label), CTORMOVE(currency), CTORMOVE(type),
        CTORMOVE(last_close_value) {}
};

inline void to_json(json &j, const Asset &v) {
  auto val = std::string();
  if (v.last_close_value) {
    json j_val = *v.last_close_value;
    val = j_val.dump();

    // Remove quotes around json str
    val = val.substr(1, val.size() - 2);
  }

  auto j_str = json::string_t(fmt::format("000{}|{}", v.id, val));

  j_str[0] = v.label.value;
  j_str[1] = v.type.value;
  j_str[2] = v.currency.value;
  j = j_str;
}

inline void from_json(const json &j, Asset &v) {
  std::string token;
  auto stream = std::istringstream(j.get<std::string>());

  v.label = AssetLabel(stream.get());
  v.type = AssetType(stream.get());
  v.currency = CurrencyCode(stream.get());

  std::getline(stream, token, '|');
  v.id = token;

  std::getline(stream, token, '|');
  if (token.empty()) {
    v.last_close_value = std::nullopt;
  } else {
    json j_val = token;
    AssetValue val;
    from_json(j_val, val);

    v.last_close_value = val;
  }
}
} // namespace CompactTypes