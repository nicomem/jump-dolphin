#include "check.hpp"
#include "jump/client.hpp"
#include "jump/types_json.hpp"
#include "save_data.hpp"
#include "stochastic.hpp"
#include "tree.hpp"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <thread>

#include <CLI/CLI.hpp>

constexpr auto VERBOSE = true;

namespace FinalPortfolio {
static auto portfolio_folder = std::filesystem::current_path() / "portfolio";
static auto best_portfolio_path = portfolio_folder / "best_portfolio.json";
static auto new_portfolio_path = portfolio_folder / "new_portfolio.json";

static JumpTypes::Portfolio load_portfolio(const std::filesystem::path &path) {
  auto f = std::ifstream(path);
  if (!f.good()) {
    std::cerr << "Could not load portfolio in '" << path << "'\n";
    exit(EXIT_FAILURE);
  }

  json j;
  f >> j;

  return j.get<JumpTypes::Portfolio>();
}

static void save_portfolio(const std::filesystem::path &path,
                           const JumpTypes::Portfolio &new_portfolio) {
  std::filesystem::create_directory(portfolio_folder);

  auto f = std::ofstream(path);
  if (!f.good()) {
    std::cerr << "Could not save portfolio in '" << path << "'\n";
    exit(EXIT_FAILURE);
  }

  f << json(new_portfolio) << '\n';
}

compo_t best_compo(const TrucsInteressants &trucs) {
  const auto portfolio =
      FinalPortfolio::load_portfolio(FinalPortfolio::best_portfolio_path);
  const auto &values = portfolio.values.find("2016-06-01")->second;

  auto compo = compo_t();
  compo.reserve(values.size());
  for (const auto &[vals_opt, _] : values) {
    auto [id, nb_shares] = *vals_opt;

    auto it = std::find(trucs.assets_id.begin(), trucs.assets_id.end(),
                        std::to_string(id));
    if (it == trucs.assets_id.end()) {
      std::cerr << "Could not find asset " << id << " in the trucs\n";
      exit(EXIT_FAILURE);
    }

    auto index = std::distance(trucs.assets_id.begin(), it);
    compo.emplace_back(nb_shares, index);
  }
  return compo;
}

JumpTypes::Portfolio to_portfolio(const TrucsInteressants &trucs,
                                  const compo_t &compo) {
  auto new_values = std::vector<JumpTypes::portfolio_value>();
  new_values.reserve(compo.size());
  for (const auto &[shares, i_asset] : compo) {
    double nb_shares = shares;
    new_values.emplace_back(
        JumpTypes::PortfolioAsset{std::stoi(trucs.assets_id[i_asset]),
                                  nb_shares},
        std::nullopt);
  }

  return {std::string("EPITA_PTF_6"),
          JumpTypes::CurrencyCode::EUR,
          JumpTypes::DynAmountType::front,
          {{std::string("2016-06-01"), new_values}}};
}

std::string get_sharpe(JumpClient &client) {
  auto ratios = std::vector<int32_t>();
  ratios.emplace_back(12);

  auto assets_id = std::vector<int32_t>();
  assets_id.emplace_back(1825);

  JumpTypes::RatioParam params = JumpTypes::RatioParam();
  params.ratio = ratios;
  params.asset = assets_id;
  params.benchmark = std::nullopt;
  params.start_date = std::string("2016-06-01");
  params.end_date = std::string("2020-09-30");

  // Get and parse the sharpes
  return client.compute_ratio(std::move(params))
      .value.find("1825")
      ->second.find("12")
      ->second.value;
}
} // namespace FinalPortfolio

#define CHECK_CORRUPTION(S1, S2)                                               \
  do {                                                                         \
    auto s1 = S1;                                                              \
    auto s2 = S2;                                                              \
    if (s1 != s2) {                                                            \
      std::cerr << "(" #S1 "= " << s1 << ") != (" #S2 "= " << s2               \
                << "): Corrupted files, please delete data/\n";                \
      exit(EXIT_FAILURE);                                                      \
    }                                                                          \
  } while (0)

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
  // auto nb_shares =
  // SaveData::start_date_assets_volumes(*filtered_days_assets,
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

  CHECK_CORRUPTION(start_values.size(), end_values.size());
  CHECK_CORRUPTION(start_values.size(), cov_matrix.size());
  CHECK_CORRUPTION(start_values.size(), nb_shares.size());
  CHECK_CORRUPTION(start_values.size(), assets_id.size());
  CHECK_CORRUPTION(start_values.size(), assets_capital.size());

  return TrucsInteressants{start_values, end_values, cov_matrix,
                           nb_shares,    assets_id,  assets_capital};
}

static void thread_worker(const TrucsInteressants &trucs,
                          std::tuple<compo_t, sharpe_t> &result,
                          unsigned i_thread) {
  auto compo = compo_t();
  compo.reserve(40);
  compo.emplace_back(-1, i_thread);

  auto shares_capital = finmath::asset_period_values_t();
  shares_capital.reserve(40);

  result = max_compo_tree2(trucs, shares_capital, compo, i_thread + 1);
}

static std::tuple<compo_t, sharpe_t>
max_compo_tree_multithread(const TrucsInteressants &trucs) {
  auto nb_threads = trucs.assets_id.size() - min_portfolio_size;

  auto threads = std::vector<std::thread>();
  threads.reserve(nb_threads);

  auto results = std::vector<std::tuple<compo_t, sharpe_t>>(nb_threads);

  for (auto i = 0u; i < nb_threads; ++i) {
    threads.emplace_back(thread_worker, std::ref(trucs), std::ref(results[i]),
                         i);
  }

  compo_t best_compo;
  sharpe_t best_sharpe = -INFINITY;

  for (auto i = 0u; i < nb_threads; ++i) {
    threads[nb_threads - i - 1].join();

    const auto &[compo, sharpe] = results[nb_threads - i - 1];
    if (sharpe > best_sharpe) {
      best_sharpe = sharpe;
      best_compo = compo;
    }

    std::clog << "\n=========\nA thread has completed\n";
    if (best_sharpe == -INFINITY) {
      std::clog << "No composition found\n";
    } else {
      std::clog << "Current best composition:\n";

      std::cout << "Sharpe: " << best_sharpe << '\n';
      std::cout << "List of (asset_id, bought_shares):\n";
      for (const auto &[nb_shares, i_asset] : best_compo) {
        std::cout << "- " << trucs.assets_id[i_asset] << "\t" << nb_shares
                  << '\n';
      }
    }
  }

  return *std::max_element(results.begin(), results.end(),
                           [](const auto &a, const auto &b) {
                             return std::get<1>(b) > std::get<1>(a);
                           });
}

/** Push the portfolio in `new_portfolio_path` and copy it to
 * `best_portfolio_path` */
static void push_portfolio(JumpClient &client,
                           const JumpTypes::Portfolio &portfolio) {
  auto p = portfolio;
  client.put_portfolio_compo(std::string("1825"), std::move(p));

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  p = portfolio;
  client.put_portfolio_compo(std::string("1825"), std::move(p));

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  auto r = client.get_asset(std::string("1825"),
                            std::make_optional(std::string("2016-06-01")));
  std::cout << "Last close value: " << r.last_close_value->value << '\n';
}

static void check_portfolio(const TrucsInteressants &trucs) {
  auto compo = FinalPortfolio::best_compo(trucs);

  check_compo(trucs, compo, true);
}

static void optimize_portfolio(const TrucsInteressants &trucs,
                               JumpClient &client) {
  auto compo = FinalPortfolio::best_compo(trucs);

  auto old_sharpe = FinalPortfolio::get_sharpe(client);

  std::cout << "---\n";
  auto get_sharpe = [&trucs, &client](const compo_t &compo) -> double {
    auto portfolio = FinalPortfolio::to_portfolio(trucs, compo);
    auto p = portfolio;
    client.put_portfolio_compo(std::string("1825"), std::move(p));

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    p = portfolio;
    client.put_portfolio_compo(std::string("1825"), std::move(p));

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    auto sharpe_str = FinalPortfolio::get_sharpe(client);
    std::cout << sharpe_str << '\n';
    std::replace(sharpe_str.begin(), sharpe_str.end(), ',', '.');
    return std::stod(sharpe_str);
  };
  auto new_compo = find_best_compo_stochastic(trucs, compo, get_sharpe);

  if (!check_compo(trucs, new_compo, true)) {
    std::cerr << "Optimized compo is not valid\n";
    exit(EXIT_FAILURE);
  }
  std::cout << "---\n";

  auto new_portfolio = FinalPortfolio::to_portfolio(trucs, new_compo);
  auto best_portfolio =
      FinalPortfolio::load_portfolio(FinalPortfolio::best_portfolio_path);

  // Save the portfolio
  FinalPortfolio::save_portfolio(FinalPortfolio::new_portfolio_path,
                                 new_portfolio);

  // Push the portfolio to compute the sharpe
  push_portfolio(client, new_portfolio);

  // Get the sharpes
  auto new_sharpe = FinalPortfolio::get_sharpe(client);

  std::cout << "Old portfolio JUMP Sharpe: " << old_sharpe << '\n';
  std::cout << "Optimized portfolio JUMP Sharpe: " << new_sharpe << '\n';

  // Re-push the old portfolio
  push_portfolio(client, best_portfolio);
}

int main(int argc, char *argv[]) {
  std::string username, password, mode;

  // Parse the command line arguments
  auto app = CLI::App{"Dolphin"};
  app.add_option("-u,--username", username, "JUMP account username")
      ->required();
  app.add_option("-p,--password", password, "JUMP account password")
      ->required();
  app.add_option("-m,--mode", mode, "The action to do")
      ->required()
      ->check(CLI::IsMember({"check", "push", "compute-brute", "optimize"}));

  CLI11_PARSE(app, argc, argv);

  // Create the JUMP API client
  auto client = JumpClient::build(std::move(username), std::move(password));

  // Load or fetch the pre-calculated data
  auto trucs = get_the_trucs_interessants(*client);

  if (mode == "check") {
    check_portfolio(trucs);
  } else if (mode == "optimize") {
    optimize_portfolio(trucs, *client);
  } else if (mode == "push") {
    auto new_portfolio =
        FinalPortfolio::load_portfolio(FinalPortfolio::new_portfolio_path);
    FinalPortfolio::save_portfolio(FinalPortfolio::best_portfolio_path,
                                   new_portfolio);
    push_portfolio(*client, new_portfolio);
    std::cout << "Sharpe: " << FinalPortfolio::get_sharpe(*client);
  } else {
    // Try to create the best portfolio
    std::clog << "max_compo_tree\n";
    auto [best_compo, best_sharpe] = max_compo_tree_multithread(trucs);

    // Display the best portfolio found
    std::cout << "\nBest portfolio found:\n";
    std::cout << "Sharpe: " << best_sharpe << '\n';
    std::cout << "List of (asset_id, bought_shares):\n";
    for (const auto &[nb_shares, i_asset] : best_compo) {
      std::cout << "- " << trucs.assets_id[i_asset] << "\t" << nb_shares
                << '\n';
    }
  }
}
