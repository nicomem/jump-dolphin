#include "save_data.hpp"

#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>

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
