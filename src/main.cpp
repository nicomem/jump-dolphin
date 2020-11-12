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

  auto days_assets = SaveData::every_days_assets(*client, VERBOSE);
}
