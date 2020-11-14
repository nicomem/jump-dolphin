#include <iostream>

#include <CLI/CLI.hpp>

#include "jump/client.hpp"
#include "jump/types_json.hpp"
#include "save_data.hpp"

#include <chrono>
#include <unordered_set>

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
  auto start_rates =
      SaveData::start_date_currency_rate(days_rates, *client, VERBOSE);
  auto end_rates =
      SaveData::end_date_currency_rate(days_rates, *client, VERBOSE);

  std::optional<SaveData::DaysAssets> days_assets = std::nullopt;
  auto start_assets =
      SaveData::assets_start_values(days_assets, start_rates, *client, VERBOSE);
  auto end_assets =
      SaveData::assets_end_values(days_assets, end_rates, *client, VERBOSE);
  auto cov_matrix =
      SaveData::covariance_matrix(days_assets, days_rates, *client, VERBOSE);

  days_assets = std::nullopt;
  days_rates = std::nullopt;
}
