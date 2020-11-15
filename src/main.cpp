#include <iostream>

#include <CLI/CLI.hpp>

#include "jump/client.hpp"
#include "jump/types_json.hpp"
#include "save_data.hpp"
#include "tree.hpp"

constexpr auto VERBOSE = true;

static TrucsInteressants get_the_trucs_interessants(JumpClient &client) {
  std::optional<finmath::days_currency_rates_t> days_rates = std::nullopt;
  std::optional<SaveData::DaysAssets> filtered_days_assets = std::nullopt;
  std::vector<finmath::nb_shares_t> nb_shares;

  {
    std::optional<SaveData::DaysAssets> every_days_assets = std::nullopt;

    std::clog << "days_assets & filter & volumes...\n";
    std::tie(filtered_days_assets, nb_shares) =
        SaveData::filtered_assets_and_volumes(every_days_assets, client,
                                              VERBOSE);
  }

  std::clog << "start_rates...\n";
  auto start_rates =
      SaveData::start_date_currency_rate(days_rates, client, VERBOSE);

  std::clog << "end_rates...\n";
  auto end_rates =
      SaveData::end_date_currency_rate(days_rates, client, VERBOSE);

  std::clog << "assets_start_values...\n";
  auto start_values = SaveData::assets_start_values(
      filtered_days_assets, start_rates, client, VERBOSE);

  std::clog << "assets_end_values...\n";
  auto end_values = SaveData::assets_end_values(filtered_days_assets, end_rates,
                                                client, VERBOSE);

  std::clog << "covariance_matrix...\n";
  auto cov_matrix = SaveData::covariance_matrix(filtered_days_assets,
                                                days_rates, client, VERBOSE);

  // std::clog << "start_date_assets_volumes...\n";
  // auto nb_shares = SaveData::start_date_assets_volumes(*filtered_days_assets,
  //                                                      client, VERBOSE);

  std::clog << "assets_id...\n";
  auto assets_id = SaveData::assets_id(*filtered_days_assets);

  // std::cout << (*days_assets)["2016-06-01"][259].id << '\n';
  // return 0;

  auto assets_capital = std::vector<double>();
  assets_capital.reserve(start_values.size());
  for (auto i = 0u; i < start_values.size(); ++i) {
    assets_capital.emplace_back(start_values[i] * nb_shares[i]);
  }

  return TrucsInteressants{start_values, end_values, cov_matrix,
                           nb_shares,    assets_id,  assets_capital};
}

int main(int argc, char *argv[]) {

  std::string username, password;

  // Parse the command line arguments
  auto app = CLI::App{"Dolphin"};
  app.add_option("-u,--username", username, "JUMP account username")
      ->required();
  app.add_option("-p,--password", password, "JUMP account password")
      ->required();
  CLI11_PARSE(app, argc, argv);

  // Create the JUMP API client
  auto client = JumpClient::build(std::move(username), std::move(password));

  // Load or fetch the pre-calculated data
  auto trucs = get_the_trucs_interessants(*client);
  auto compo = compo_t();
  compo.reserve(40);

  auto shares_capital = finmath::asset_period_values_t();
  shares_capital.reserve(40);

  // Try to create the best portfolio
  std::clog << "max_compo_tree\n";
  auto [best_compo, best_sharpe] =
      max_compo_tree2(trucs, shares_capital, compo);

  // Display the best portfolio found
  std::cout << "\nBest portfolio found:\n";
  std::cout << "Sharpe: " << best_sharpe << '\n';
  std::cout << "List of (bought_shares, asset_id):\n";
  for (const auto &[nb_shares, i_asset] : best_compo) {
    std::cout << "- " << nb_shares << "\t" << trucs.assets_id[i_asset] << '\n';
  }
}
