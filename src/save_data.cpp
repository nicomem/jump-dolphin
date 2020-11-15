#include "save_data.hpp"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>
#include <thread>

/** Helper to create the method getter. Will only create the function
 * declaration, the body must be added just after the call */
#define IMPL_GETTER(RET, FUN)                                                  \
  static RET FUN##_getter(JumpClient &client, bool verbose)

/** Helper to automatically create load or get and save methods.
 * A (FUN)_getter must exists */
#define IMPL_METHOD(RET, FUN)                                                  \
  RET SaveData::FUN(JumpClient &client, bool verbose) {                        \
    constexpr std::string_view fname = #FUN ".json";                           \
    auto getter = [&client, verbose]() {                                       \
      return FUN##_getter(client, verbose);                                    \
    };                                                                         \
    return load_or_download<RET>(fname, getter);                               \
  }

#define IMPL_OPT_GETTER(RET, T, FUN)                                           \
  static RET FUN##_getter(std::optional<T> &opt_data, JumpClient &client,      \
                          bool verbose)

/** Helper to automatically create load or get and save methods.
 * A (FUN)_getter must exists */
#define IMPL_OPT_METHOD(RET, T, FUN)                                           \
  RET SaveData::FUN(std::optional<T> &opt_data, JumpClient &client,            \
                    bool verbose) {                                            \
    constexpr std::string_view fname = #FUN ".json";                           \
    auto getter = [&opt_data, &client, verbose]() {                            \
      return FUN##_getter(opt_data, client, verbose);                          \
    };                                                                         \
    return load_or_download<RET>(fname, getter);                               \
  }

#define IMPL_GETTER2(RET, T, FUN)                                              \
  static RET FUN##_getter(std::optional<T> &opt_data,                          \
                          const finmath::day_currency_rates_t &rates,          \
                          JumpClient &client, bool verbose)

/** Helper to automatically create load or get and save methods.
 * A (FUN)_getter must exists */
#define IMPL_METHOD2(RET, T, FUN)                                              \
  RET SaveData::FUN(std::optional<T> &opt_data,                                \
                    const finmath::day_currency_rates_t &rates,                \
                    JumpClient &client, bool verbose) {                        \
    constexpr std::string_view fname = #FUN ".json";                           \
    auto getter = [&opt_data, &rates, &client, verbose]() {                    \
      return FUN##_getter(opt_data, rates, client, verbose);                   \
    };                                                                         \
    return load_or_download<RET>(fname, getter);                               \
  }

#define IMPL_GETTER3(RET, FUN)                                                 \
  static RET FUN##_getter(                                                     \
      std::optional<SaveData::DaysAssets> &assets,                             \
      std::optional<finmath::days_currency_rates_t> &rates,                    \
      JumpClient &client, bool verbose)

/** Helper to automatically create load or get and save methods.
 * A (FUN)_getter must exists */
#define IMPL_METHOD3(RET, FUN)                                                 \
  RET SaveData::FUN(std::optional<SaveData::DaysAssets> &assets,               \
                    std::optional<finmath::days_currency_rates_t> &rates,      \
                    JumpClient &client, bool verbose) {                        \
    constexpr std::string_view fname = #FUN ".json";                           \
    auto getter = [&assets, &rates, &client, verbose]() {                      \
      return FUN##_getter(assets, rates, client, verbose);                     \
    };                                                                         \
    return load_or_download<RET>(fname, getter);                               \
  }

#define IMPL_GETTER4(RET, FUN)                                                 \
  static RET FUN##_getter(const SaveData::DaysAssets &days_assets,             \
                          JumpClient &client, bool verbose)

/** Helper to automatically create load or get and save methods.
 * A (FUN)_getter must exists */
#define IMPL_METHOD4(RET, FUN)                                                 \
  RET SaveData::FUN(const SaveData::DaysAssets &days_assets,                   \
                    JumpClient &client, bool verbose) {                        \
    constexpr std::string_view fname = #FUN ".json";                           \
    auto getter = [&days_assets, &client, verbose]() {                         \
      return FUN##_getter(days_assets, client, verbose);                       \
    };                                                                         \
    return load_or_download<RET>(fname, getter);                               \
  }

#define IMPL_GETTER5(RET, FUN)                                                 \
  static RET FUN##_getter(std::optional<SaveData::DaysAssets> &days_assets,    \
                          JumpClient &client, bool verbose)

/** Helper to automatically create load or get and save methods.
 * A (FUN)_getter must exists */
#define IMPL_METHOD5(RET, FUN)                                                 \
  RET SaveData::FUN(std::optional<SaveData::DaysAssets> &days_assets,          \
                    JumpClient &client, bool verbose) {                        \
    constexpr std::string_view fname = #FUN ".json";                           \
    auto getter = [&days_assets, &client, verbose]() {                         \
      return FUN##_getter(days_assets, client, verbose);                       \
    };                                                                         \
    return load_or_download<RET>(fname, getter);                               \
  }

template <class T> static void save(std::string_view fname, T data) {
  static auto root = std::filesystem::current_path() / "data";
  std::filesystem::create_directory(root);

  auto f = std::ofstream(root / fname);
  if (!f.good()) {
    std::cerr << "Could not save to " << root / fname << "\n";
    return;
  }
  json j = data;
  f << j << '\n';
}

template <class T> static std::optional<T> load(std::string_view fname) {
  static auto root = std::filesystem::current_path() / "data";

  auto f = std::ifstream(root / fname);
  if (!f.good()) {
    return std::nullopt;
  }

  json j;
  f >> j;
  try {
    return j.get<T>();
  } catch (const std::exception &e) {
    return std::nullopt;
  }
}

template <typename T>
static T load_or_download(std::string_view fname, std::function<T()> getter) {
  // Try to load
  auto r = load<T>(fname);
  if (r.has_value())
    return r.value();

  // If cannot load, download and save
  T v = getter();
  save(fname, v);
  return v;
}

IMPL_GETTER(SaveData::DaysAssets, every_days_assets) {
  using namespace date;
  constexpr auto date_start = sys_days(2016_y / June / 1);
  constexpr auto date_end = sys_days(2020_y / September / 30);

  auto map = SaveData::DaysAssets();

  auto time_start = std::chrono::system_clock::now();
  auto nb_errors = 0;

  if (verbose) {
    std::clog << "Fetching all assets every day between " << date_start
              << " and " << date_end << '\n';
  }

  for (auto date = date_start; date <= date_end; date += days{1}) {
    if (verbose) {
      // Log the progress periodically
      auto dt = std::chrono::system_clock::now() - time_start;
      if (dt > std::chrono::seconds(5)) {
        std::clog << "Date: " << date << " | Date errors: " << nb_errors
                  << '\n';
        time_start = std::chrono::system_clock::now();
      }
    }

    // Convert the date to string
    auto date_stream = std::stringstream("");
    date_stream << date;

    try {
      // Try to get the assets and add them to the vector
      auto date_str = date_stream.str();
      auto assets = client.get_assets(date_str);

      // Create a compact version of the assets
      auto compact = std::vector<CompactTypes::Asset>();
      compact.reserve(assets.size());
      for (auto &&asset : std::move(assets)) {
        compact.emplace_back(std::move(asset));
      }

      // Add it to the map
      map.emplace(std::move(date_str), std::move(compact));
    } catch (const std::exception &e) {
      // If an error happened, continue with the next day
      ++nb_errors;
      continue;
    }
  }

  return map;
}
IMPL_METHOD(SaveData::DaysAssets, every_days_assets)

IMPL_GETTER5(SaveData::DaysAssetAndVolumes, filtered_assets_and_volumes) {
  if (!days_assets) {
    days_assets = SaveData::every_days_assets(client, verbose);
  }

  const auto &first_day_assets = (*days_assets)["2016-06-01"];
  const auto &last_day_assets = (*days_assets)["2020-09-30"];
  auto stock_index = std::vector<unsigned>();
  auto volumes = std::vector<finmath::nb_shares_t>();

  // Filter the assets to keep
  for (unsigned i = 0; i < first_day_assets.size(); ++i) {
    // Remove non-stock assets
    if (first_day_assets[i].type.value != CompactTypes::AssetType::STOCK)
      continue;

    // Remove assets that do not have buy/sell values
    if (!first_day_assets[i].last_close_value.has_value() ||
        !last_day_assets[i].last_close_value.has_value())
      continue;

    // Remove assets that have a negative return
    if (first_day_assets[i].last_close_value.value().value >=
        last_day_assets[i].last_close_value.value().value)
      continue;

    // Get the volume for the asset
    auto id = first_day_assets[i].id;
    auto quotes = client.get_asset_quote(
        std::move(id), std::make_optional(std::string("2016-06-01")),
        std::make_optional(std::string("2016-06-01")));

    // If nothing can be bought, the asset is not interesting
    if (quotes.empty() || quotes[0].volume == 0) {
      continue;
    } else {
      volumes.emplace_back(quotes[0].volume);
    }

    stock_index.emplace_back(i);
  }

  if (verbose) {
    std::clog << "Interesting assets: " << stock_index.size() << " / "
              << first_day_assets.size() << std::endl;
  }

  if (verbose) {
    // Keep only the assets that have the best sharpes
    std::clog << "Filter based on sharpe...\n";
  }

  // Get the sharpes
  auto ratios = std::vector<int32_t>();
  ratios.emplace_back(12);

  auto assets_id = std::vector<int32_t>();
  for (auto i : stock_index) {
    const auto &asset_id = first_day_assets[i].id;
    assets_id.emplace_back(std::stoi(asset_id));
  }

  JumpTypes::RatioParam params = JumpTypes::RatioParam();
  params.ratio = ratios;
  params.asset = assets_id;
  params.benchmark = std::nullopt;
  params.start_date = std::string("2016-06-01");
  params.end_date = std::string("2020-09-30");

  // Get and parse the sharpes
  auto ratios_sharpe = client.compute_ratio(std::move(params));

  // Remove assets that have low sharpe
  auto j = 0u;
  for (auto i = 0u; i < stock_index.size(); ++i) {
    const auto &asset_id = first_day_assets[stock_index[i]].id;
    auto sharpe_str =
        ratios_sharpe.value.find(asset_id)->second.find("12")->second.value;
    std::replace(sharpe_str.begin(), sharpe_str.end(), ',', '.');
    auto sharpe = std::stod(sharpe_str);

    if (sharpe >= 0.5) {
      stock_index[j] = stock_index[i];
      ++j;
    }
  }

  auto last_size = stock_index.size();
  stock_index.resize(j);

  if (verbose) {
    std::clog << "2nd filtering: " << j << " / " << last_size << std::endl;
  }

  // Create a new DayAsset with only the interesting assets
  auto res = SaveData::DaysAssets();
  for (const auto &[date, day_asset] : *days_assets) {
    auto day_vec = std::vector<CompactTypes::Asset>();
    day_vec.reserve(stock_index.size());
    for (auto i : stock_index) {
      day_vec.push_back(day_asset[i]);
    }
    res.emplace(date, day_vec);
  }

  // Also remove the assets volumes
  auto volumes2 = std::vector<finmath::nb_shares_t>();
  volumes2.reserve(stock_index.size());
  for (auto i : stock_index) {
    volumes2.push_back(volumes[i]);
  }

  return std::make_tuple(res, volumes2);
}
IMPL_METHOD5(SaveData::DaysAssetAndVolumes, filtered_assets_and_volumes)

std::vector<std::string> SaveData::assets_id(const DaysAssets &days_assets) {
  const auto &first_day_assets = days_assets.find("2016-06-01")->second;

  auto ids = std::vector<std::string>();
  ids.reserve(first_day_assets.size());

  for (const auto &asset : first_day_assets) {
    ids.emplace_back(asset.id);
  }

  return ids;
}

IMPL_GETTER2(finmath::assets_day_values_t, SaveData::DaysAssets,
             assets_start_values) {
  using namespace date;
  constexpr auto date_start = sys_days(2016_y / June / 1);

  // Convert the date to string
  auto date_stream = std::stringstream("");
  date_stream << date_start;

  if (!opt_data) {
    opt_data = SaveData::every_days_assets(client, verbose);
  }

  const auto &day_assets = (*opt_data)[date_stream.str()];

  auto r = finmath::assets_day_values_t();
  r.reserve(day_assets.size());

  for (const auto &asset : day_assets) {
    if (asset.last_close_value) {
      auto rate = rates[JumpTypes::index(asset.currency.to_jump())];
      r.emplace_back(asset.last_close_value->value * rate);
    } else {
      r.emplace_back(-1);
    }
  }

  return r;
}
IMPL_METHOD2(finmath::assets_day_values_t, SaveData::DaysAssets,
             assets_start_values)

IMPL_GETTER2(finmath::assets_day_values_t, SaveData::DaysAssets,
             assets_end_values) {
  using namespace date;
  constexpr auto date_end = sys_days(2020_y / September / 30);

  // Convert the date to string
  auto date_stream = std::stringstream("");
  date_stream << date_end;

  if (!opt_data) {
    opt_data = SaveData::every_days_assets(client, verbose);
  }

  const auto &day_assets = (*opt_data)[date_stream.str()];

  auto r = finmath::assets_day_values_t();
  r.reserve(day_assets.size());

  for (const auto &asset : day_assets) {
    if (asset.last_close_value) {
      auto rate = rates[JumpTypes::index(asset.currency.to_jump())];
      r.emplace_back(asset.last_close_value->value * rate);
    } else {
      r.emplace_back(-1);
    }
  }

  return r;
}
IMPL_METHOD2(finmath::assets_day_values_t, SaveData::DaysAssets,
             assets_end_values)

static double get_mean(unsigned int i_asset, SaveData::DaysAssets &assets) {
  // mean
  double last_val = 0;
  double sum = 0;
  for (const auto &[date, day_assets] : assets) {
    const auto &val = day_assets[i_asset].last_close_value;
    double v = (val ? val->value : last_val);

    sum += v;
    last_val = v;
  }
  auto mean = sum / assets.size();
  return mean;
}

static double get_cov(unsigned int i_asset, unsigned int j_asset,
                      SaveData::DaysAssets &assets) {
  // get mean
  auto mean_i = get_mean(i_asset, assets);
  auto mean_j = get_mean(j_asset, assets);

  // Compute cov
  double cov = 0;
  double last_val1 = 0;
  double last_val2 = 0;
  for (const auto &[date, day_assets] : assets) {
    last_val1 = (day_assets[i_asset].last_close_value
                     ? day_assets[i_asset].last_close_value->value
                     : last_val1);
    last_val2 = (day_assets[j_asset].last_close_value
                     ? day_assets[j_asset].last_close_value->value
                     : last_val2);

    cov += (last_val1 - mean_i) * (last_val2 - mean_j);
  }
  return cov;
}

IMPL_GETTER3(finmath::covariance_matrix_t, covariance_matrix) {
  if (!assets) {
    assets = SaveData::every_days_assets(client, verbose);
  }

  if (!rates) {
    rates = SaveData::days_currency_rates(client, verbose);
  }

  auto asset_size = (*assets)["2016-06-01"].size();

  auto cov_matrix = finmath::covariance_matrix_t();
  cov_matrix.reserve(asset_size);

  // get volatilities
  auto ratios = std::vector<int32_t>();
  ratios.emplace_back(10);

  auto assets_id = std::vector<int32_t>();
  const auto &first_day_assets = (*assets)["2016-06-01"];
  for (auto i_asset = 0u; i_asset < asset_size; ++i_asset) {
    auto asset_id = first_day_assets[i_asset].id;
    assets_id.emplace_back(std::stoi(asset_id));
  }

  JumpTypes::RatioParam params = JumpTypes::RatioParam();
  params.ratio = ratios;
  params.asset = assets_id;
  params.benchmark = std::nullopt;
  params.start_date = std::string("2016-06-01");
  params.end_date = std::string("2020-09-30");

  auto ratios_volatilities = client.compute_ratio(std::move(params));
  auto vars = std::vector<double>();
  vars.reserve(asset_size);
  for (auto i_asset = 0u; i_asset < asset_size; ++i_asset) {
    vars.emplace_back(get_cov(i_asset, i_asset, *assets));
  }

  // correlation
  for (auto i_asset = 0u; i_asset < asset_size; ++i_asset) {
    auto &cov_vec = cov_matrix.emplace_back();
    double var_i = vars[i_asset];

    if (verbose) {
      std::clog << "Correlation: " << i_asset << '/' << asset_size << '\n';
    }

    for (auto j_asset = 0u; j_asset < asset_size; ++j_asset) {
      double cov = get_cov(i_asset, j_asset, *assets);
      double var_j = vars[j_asset];

      double correlation = cov / std::sqrt(var_i * var_j);

      double vol_i = 0;
      double vol_j = 0;

      try {
        auto vol_str_i =
            ratios_volatilities.value[first_day_assets[i_asset].id]["10"].value;
        std::replace(vol_str_i.begin(), vol_str_i.end(), ',', '.');
        vol_i = std::stod(vol_str_i);

        auto vol_str_j =
            ratios_volatilities.value[first_day_assets[j_asset].id]["10"].value;
        std::replace(vol_str_j.begin(), vol_str_j.end(), ',', '.');
        vol_j = std::stod(vol_str_j);
      } catch (std::exception &e) {
        cov_vec.emplace_back(0);
        continue;
      }

      double result = correlation * std::sqrt(vol_i * vol_j);

      cov_vec.emplace_back(result);
    }
  }
  return cov_matrix;
}
IMPL_METHOD3(finmath::covariance_matrix_t, covariance_matrix)

IMPL_GETTER(finmath::days_currency_rates_t, days_currency_rates) {
  using namespace date;
  constexpr auto date_start = sys_days(2016_y / June / 1);
  constexpr auto date_end = sys_days(2020_y / September / 30);

  auto map = finmath::days_currency_rates_t();

  auto time_start = std::chrono::system_clock::now();
  auto nb_errors = 0;

  if (verbose) {
    std::clog << "Fetching rates every day between " << date_start << " and "
              << date_end << '\n';
  }

  for (auto date = date_start; date <= date_end; date += days{1}) {
    if (verbose) {
      // Log the progress periodically
      auto dt = std::chrono::system_clock::now() - time_start;
      if (dt > std::chrono::seconds(5)) {
        std::clog << "Date: " << date << " | Date errors: " << nb_errors
                  << '\n';
        time_start = std::chrono::system_clock::now();
      }
    }

    // Convert the date to string
    auto date_stream = std::stringstream("");
    date_stream << date;

    std::this_thread::sleep_for(
        std::chrono::milliseconds(50)); // because the API sucks
    try {
      // Try to get the assets and add them to the vector
      auto date_str = date_stream.str();

      auto res = finmath::day_currency_rates_t();

      for (auto currency : JumpTypes::currencies) {
        auto date_stream = std::stringstream("");
        date_stream << date;

        double rate = client.get_currency_change_rate(
            currency, JumpTypes::CurrencyCode::EUR,
            std::make_optional(date_stream.str()));

        res[JumpTypes::index(currency)] = rate;
      }

      // Add it to the map
      map.emplace(std::move(date_str), std::move(res));
    } catch (const std::exception &e) {
      // If an error happened, continue with the next day
      std::cerr << e.what() << std::endl;
      ++nb_errors;
      continue;
    }
  }

  return map;
}
IMPL_METHOD(finmath::days_currency_rates_t, days_currency_rates)

IMPL_OPT_GETTER(finmath::day_currency_rates_t, finmath::days_currency_rates_t,
                start_date_currency_rate) {
  if (!opt_data) {
    opt_data = SaveData::days_currency_rates(client, verbose);
  }

  using namespace date;
  constexpr auto date_start = sys_days(2016_y / June / 1);

  // Convert the date to string
  auto date_stream = std::stringstream("");
  date_stream << date_start;

  return (*opt_data)[date_stream.str()];
}
IMPL_OPT_METHOD(finmath::day_currency_rates_t, finmath::days_currency_rates_t,
                start_date_currency_rate)

IMPL_OPT_GETTER(finmath::day_currency_rates_t, finmath::days_currency_rates_t,
                end_date_currency_rate) {
  if (!opt_data) {
    opt_data = SaveData::days_currency_rates(client, verbose);
  }

  using namespace date;
  constexpr auto date_end = sys_days(2020_y / September / 30);

  // Convert the date to string
  auto date_stream = std::stringstream("");
  date_stream << date_end;

  return (*opt_data)[date_stream.str()];
}
IMPL_OPT_METHOD(finmath::day_currency_rates_t, finmath::days_currency_rates_t,
                end_date_currency_rate)

IMPL_GETTER4(std::vector<finmath::nb_shares_t>, start_date_assets_volumes) {
  (void)verbose;
  const auto &first_day_assets = days_assets.find("2016-06-01")->second;

  auto volumes = std::vector<finmath::nb_shares_t>();
  volumes.reserve(first_day_assets.size());

  for (const auto &asset : first_day_assets) {
    auto id = asset.id;
    auto quotes = client.get_asset_quote(
        std::move(id), std::make_optional(std::string("2016-06-01")),
        std::make_optional(std::string("2016-06-01")));

    if (quotes.empty()) {
      volumes.emplace_back(0);
    } else {
      volumes.emplace_back(quotes[0].volume);
    }
  }

  return volumes;
}
IMPL_METHOD4(std::vector<finmath::nb_shares_t>, start_date_assets_volumes)
