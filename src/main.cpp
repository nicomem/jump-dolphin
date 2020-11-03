#include <iostream>

#include <CLI/CLI.hpp>

#include "jump/client.hpp"

// for convenience
using json = nlohmann::json;

int main(int argc, char *argv[]) {
  auto app = CLI::App{"Dolphin"};

  std::string username, password;

  app.add_option("-u,--username", username, "JUMP account username")
      ->required();
  app.add_option("-p,--password", password, "JUMP account password")
      ->required();

  CLI11_PARSE(app, argc, argv);

  auto client = JumpClient(std::move(username), std::move(password));
  auto assets = client.get_assets();
  json j = assets[0];
  std::cout << j.dump(2) << std::endl;

  auto ratios = client.get_ratios();
  j = ratios[0];
  std::cout << j.dump(2) << std::endl;
}
