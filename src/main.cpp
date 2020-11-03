#include <iostream>

#include <CLI/CLI.hpp>

#include "jump/client.hpp"

// for convenience
using json = nlohmann::json;

int main(int argc, char *argv[]) {
  char *arg1 = argv[1];
  char *arg2 = argv[2];

  auto client = JumpClient(arg1, arg2);
  auto ratios = client.get_assets();
  json j = ratios[0];
  std::cout << j.dump(2) << std::endl;
}
