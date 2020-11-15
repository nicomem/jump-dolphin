#include <iostream>

#include <CLI/CLI.hpp>

#include "jump/client.hpp"
#include "jump/types_json.hpp"
#include "save_data.hpp"
#include "tree.hpp"

#include <chrono>
#include <unordered_set>

static SaveData::DaysAssets filter_assets(SaveData::DaysAssets &assets) {
  const auto &first_day_assets = assets["2016-06-01"];
  const auto &last_day_assets = assets["2020-09-30"];
  auto stock_index = std::vector<unsigned>();

  for (unsigned i = 0; i < first_day_assets.size(); ++i) {
    if (first_day_assets[i].type.value != CompactTypes::AssetType::STOCK)
      continue;
    if (!first_day_assets[i].last_close_value.has_value() || !last_day_assets[i].last_close_value.has_value() ||
        first_day_assets[i].last_close_value.value().value >= last_day_assets[i].last_close_value.value().value)
        continue;

    stock_index.emplace_back(i);
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
  std::clog << res.size() << std::endl;

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
  auto start_values =
      SaveData::assets_start_values(days_assets, start_rates, *client, VERBOSE);
  std::clog << "assets_end_values...\n";
  auto end_values =
      SaveData::assets_end_values(days_assets, end_rates, *client, VERBOSE);
  std::clog << "covariance_matrix...\n";
  auto cov_matrix =
      SaveData::covariance_matrix(days_assets, days_rates, *client, VERBOSE);

  std::clog << "start_date_assets_volumes...\n";
  auto nb_shares =
      SaveData::start_date_assets_volumes(*days_assets, *client, VERBOSE);

  // std::cout << (*days_assets)["2016-06-01"][259].id << '\n';
  // return 0;

  days_assets = std::nullopt;
  days_rates = std::nullopt;

  auto trucs = TrucsInteressants{
      start_rates, end_rates, start_values, end_values, cov_matrix, nb_shares,
  };

  auto compo = compo_t();

  std::clog << "max_compo_tree\n";
  auto [best_compo, best_sharpe] = max_compo_tree(trucs, compo, 0.25);

  std::cout << "----\n";
  for (const auto &[nb_shares, i_asset] : best_compo) {
    std::cout << nb_shares << ' ' << i_asset << '\n';
  }

  std::cout << best_sharpe << '\n';
}
