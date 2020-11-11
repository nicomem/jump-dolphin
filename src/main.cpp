#include <iostream>

#include <CLI/CLI.hpp>

#include "jump/client.hpp"
#include "jump/types_json.hpp"

int main(int argc, char *argv[]) {
  auto app = CLI::App{"Dolphin"};

  std::string username, password;

  app.add_option("-u,--username", username, "JUMP account username")
      ->required();
  app.add_option("-p,--password", password, "JUMP account password")
      ->required();

  CLI11_PARSE(app, argc, argv);

  auto client = JumpClient::build(std::move(username), std::move(password));
  auto rate = client->get_currency_change_rate(
      "JPY", "EUR", std::make_optional("2017-01-08"));
  std::cout << rate << '\n';

  rate = client->get_currency_change_rate("JPY", "EUR",
                                          std::make_optional("2017-08-01"));
  std::cout << rate << '\n';
}
