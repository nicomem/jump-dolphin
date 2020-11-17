#include "check.hpp"
#include "jump/client.hpp"
#include "jump/types_json.hpp"
#include "save_data.hpp"
#include "tree.hpp"

#include <cassert>
#include <iostream>
#include <thread>

#include <CLI/CLI.hpp>

constexpr auto VERBOSE = true;

namespace FinalPortfolio {
static std::vector<JumpTypes::portfolio_value> values = {
    {JumpTypes::PortfolioAsset{1917, 441}, std::nullopt},
    {JumpTypes::PortfolioAsset{1918, 487}, std::nullopt},
    {JumpTypes::PortfolioAsset{1930, 126}, std::nullopt},
    {JumpTypes::PortfolioAsset{1931, 21}, std::nullopt},
    {JumpTypes::PortfolioAsset{1932, 78}, std::nullopt},
    {JumpTypes::PortfolioAsset{1934, 25}, std::nullopt},
    {JumpTypes::PortfolioAsset{1956, 231}, std::nullopt},
    {JumpTypes::PortfolioAsset{1958, 15}, std::nullopt},
    {JumpTypes::PortfolioAsset{2082, 87}, std::nullopt},
    {JumpTypes::PortfolioAsset{1963, 148}, std::nullopt},
    {JumpTypes::PortfolioAsset{1965, 16}, std::nullopt},
    {JumpTypes::PortfolioAsset{1966, 298}, std::nullopt},
    {JumpTypes::PortfolioAsset{1971, 39}, std::nullopt},
    {JumpTypes::PortfolioAsset{1973, 561}, std::nullopt},
    {JumpTypes::PortfolioAsset{1987, 182}, std::nullopt},
    {JumpTypes::PortfolioAsset{1989, 521}, std::nullopt},
    {JumpTypes::PortfolioAsset{1990, 52}, std::nullopt},
    {JumpTypes::PortfolioAsset{1995, 40}, std::nullopt},
    {JumpTypes::PortfolioAsset{2000, 90}, std::nullopt},
    {JumpTypes::PortfolioAsset{2001, 127}, std::nullopt},
    {JumpTypes::PortfolioAsset{1507, 136}, std::nullopt},
    {JumpTypes::PortfolioAsset{2090, 25}, std::nullopt},
    {JumpTypes::PortfolioAsset{2011, 85}, std::nullopt},
    {JumpTypes::PortfolioAsset{2015, 76}, std::nullopt},
    {JumpTypes::PortfolioAsset{2033, 735}, std::nullopt},
    {JumpTypes::PortfolioAsset{2034, 281}, std::nullopt},
    {JumpTypes::PortfolioAsset{2035, 2581}, std::nullopt},
    {JumpTypes::PortfolioAsset{2046, 45}, std::nullopt},
    {JumpTypes::PortfolioAsset{2023, 5378}, std::nullopt},
    {JumpTypes::PortfolioAsset{1571, 27}, std::nullopt},
    {JumpTypes::PortfolioAsset{2100, 16}, std::nullopt},
    {JumpTypes::PortfolioAsset{1595, 13}, std::nullopt},
    {JumpTypes::PortfolioAsset{1603, 15}, std::nullopt},
    {JumpTypes::PortfolioAsset{1619, 525}, std::nullopt},
    {JumpTypes::PortfolioAsset{2110, 24}, std::nullopt},
    {JumpTypes::PortfolioAsset{2112, 53}, std::nullopt},
    {JumpTypes::PortfolioAsset{1731, 30}, std::nullopt},
    {JumpTypes::PortfolioAsset{1756, 36}, std::nullopt},
    {JumpTypes::PortfolioAsset{2120, 95}, std::nullopt},
    {JumpTypes::PortfolioAsset{2106, 52}, std::nullopt},
};
static JumpTypes::Portfolio portfolio = {std::string("EPITA_PTF_6"),
                                         JumpTypes::CurrencyCode::EUR,
                                         JumpTypes::DynAmountType::front,
                                         {{std::string("2016-06-01"), values}}};
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

static void push_portfolio(JumpClient &client) {
  auto p = FinalPortfolio::portfolio;
  client.put_portfolio_compo(std::string("1825"), std::move(p));

  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  p = FinalPortfolio::portfolio;
  client.put_portfolio_compo(std::string("1825"), std::move(p));

  auto r = client.get_asset(std::string("1825"),
                            std::make_optional(std::string("2016-06-01")));
  std::cout << "Last close value: " << r.last_close_value->value << '\n';

  // Get the sharpes
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
  auto ratios_sharpe = client.compute_ratio(std::move(params));
  json sharpe_j = ratios_sharpe;

  std::cout << "Sharpe: " << sharpe_j.dump(2) << '\n';
}

static void check_portfolio(const TrucsInteressants &trucs) {
  // Build the composition from the portfolio values
  auto compo = compo_t();
  compo.reserve(FinalPortfolio::values.size());
  for (const auto &[vals_opt, _] : FinalPortfolio::values) {
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

  check_compo(trucs, compo, true);
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
      ->check(CLI::IsMember({"check", "push", "compute"}));

  CLI11_PARSE(app, argc, argv);

  // Create the JUMP API client
  auto client = JumpClient::build(std::move(username), std::move(password));

  // Load or fetch the pre-calculated data
  auto trucs = get_the_trucs_interessants(*client);

  if (mode == "check") {
    check_portfolio(trucs);
  } else if (mode == "push") {
    push_portfolio(*client);
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
