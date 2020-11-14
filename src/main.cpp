#include <iostream>

#include <CLI/CLI.hpp>

#include "jump/client.hpp"
#include "jump/types_json.hpp"
#include "save_data.hpp"

#include <chrono>
#include <unordered_set>

static SaveData::DaysAssets filter_assets(SaveData::DaysAssets &assets) {
  const auto &first_day_assets = assets["2016-06-01"];
  auto stock_index = std::vector<unsigned>();
  for (unsigned i = 0; i < first_day_assets.size(); ++i) {
    if (first_day_assets[i].type.value == CompactTypes::AssetType::STOCK) {
      stock_index.emplace_back(i);
    }
  }

  auto res = SaveData::DaysAssets();
  for (const auto &[date, day_asset] : assets) {
    auto day_vec = std::vector<CompactTypes::Asset>();
    day_vec.reserve(stock_index.size());
    for (auto i : stock_index) {
      day_vec.push_back(day_asset[i]);
    }
    res.emplace(date, day_vec);
  }

  return res;
}

int main(int argc, char *argv[]) {
  constexpr auto VERBOSE = true;

  auto app = CLI::App{"Dolphin"};

  std::string username, password;

  app.add_option("-u,--username", username, "JUMP account username")
      ->required();
  app.add_option("-p,--password", password, "JUMP account password")
      ->required();

  CLI11_PARSE(app, argc, argv);

  auto client = JumpClient::build(std::move(username), std::move(password));

  std::optional<finmath::days_currency_rates_t> days_rates = std::nullopt;

  std::clog << "start_rates...\n";
  auto start_rates =
      SaveData::start_date_currency_rate(days_rates, *client, VERBOSE);
  std::clog << "end_rates...\n";
  auto end_rates =
      SaveData::end_date_currency_rate(days_rates, *client, VERBOSE);

  std::optional<SaveData::DaysAssets> days_assets = std::nullopt;
  std::clog << "days_assets...\n";
  days_assets =
      std::make_optional(SaveData::every_days_assets(*client, VERBOSE));
  std::clog << "filter_assets...\n";
  days_assets = filter_assets(*days_assets);

  std::clog << "assets_start_values...\n";
  auto start_assets =
      SaveData::assets_start_values(days_assets, start_rates, *client, VERBOSE);
  std::clog << "assets_end_values...\n";
  auto end_assets =
      SaveData::assets_end_values(days_assets, end_rates, *client, VERBOSE);
  std::clog << "covariance_matrix...\n";
  auto cov_matrix =
      SaveData::covariance_matrix(days_assets, days_rates, *client, VERBOSE);

  days_assets = std::nullopt;
  days_rates = std::nullopt;
}
