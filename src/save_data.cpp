#include "save_data.hpp"

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

  // Compute mean for each asset
  auto assets_mean = std::vector<double>();
  assets_mean.reserve(asset_size);
  for (auto i_asset = 0u; i_asset < asset_size; ++i_asset) {
    double sum = 0;
    double last_val = 0;
    auto asset_id = std::string();
    for (const auto &[date, day_assets] : *assets) {
      asset_id = day_assets[i_asset].id;
      const auto &val = day_assets[i_asset].last_close_value;
      double v = (val ? val->value : last_val);

      sum += v;
      last_val = v;
    }
    std::cout << asset_id << ' ' << sum / assets->size() << std::endl;
    assets_mean.emplace_back(sum / assets->size());
  }

  // Compute cov matrix
  for (auto i_asset = 0u; i_asset < asset_size; ++i_asset) {
    auto &cov_vec = cov_matrix.emplace_back();
    cov_vec.reserve(asset_size);

    for (auto j_asset = 0u; j_asset < asset_size; ++j_asset) {
      double cov = 0;

      double last_val1 = 0;
      double last_val2 = 0;
      for (const auto &[date, day_assets] : *assets) {
        last_val1 = (day_assets[i_asset].last_close_value
                         ? day_assets[i_asset].last_close_value->value
                         : last_val1);
        last_val2 = (day_assets[j_asset].last_close_value
                         ? day_assets[j_asset].last_close_value->value
                         : last_val2);

        cov += (last_val1 - assets_mean[i_asset]) *
               (last_val2 - assets_mean[j_asset]);
      }

      cov_vec.emplace_back(cov / assets->size());
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
