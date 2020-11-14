#include "private_client.hpp"

#include "types_json.hpp"

#include <algorithm>
#include <cmath>
#include <tuple>

#define OPT_STR(S) std::make_optional(std::string(S))

using ParamOptKV =
    std::tuple<std::string_view, PrivateJumpClient::OptionalParameter>;

static cpr::Parameters
build_parameters(std::initializer_list<ParamOptKV> &&opt_params) {
  auto params = cpr::Parameters();

  for (auto &&[key, value] : opt_params) {
    // Only add the parameter if the value is present
    if (value.has_value()) {
      params.Add({std::string(key), value.value()});
    }
  }

  return params;
}

std::unique_ptr<JumpClient> JumpClient::build(std::string &&username,
                                              std::string &&password) {
  return std::make_unique<PrivateJumpClient>(std::move(username),
                                             std::move(password));
}

PrivateJumpClient::PrivateJumpClient(std::string &&username,
                                     std::string &&password)
    : session_(), cache_url_() {
  auto auth = cpr::Authentication{std::move(username), std::move(password)};
  session_.SetAuth(auth);
  session_.SetVerifySsl(false);
}

template <typename... Args>
static cpr::Url build_url(fmt::memory_buffer &buffer,
                          std::string_view format_str, Args... args) {
  buffer.clear();
  fmt::format_to(buffer, format_str, args...);
  return cpr::Url{buffer.data(), buffer.size()};
}

#include <iostream>
std::vector<JumpTypes::Asset>
PrivateJumpClient::get_assets(OptionalParameter date) {
  auto url = build_url(cache_url_, "{}/asset", HOST_URL);
  auto params = build_parameters({
      std::tuple{"date", date},
      std::tuple{"columns", OPT_STR("ASSET_DATABASE_ID")},
      std::tuple{"columns", OPT_STR("LABEL")},
      std::tuple{"columns", OPT_STR("LAST_CLOSE_VALUE_IN_CURR")},
      std::tuple{"columns", OPT_STR("TYPE")},
      std::tuple{"columns", OPT_STR("CURRENCY")},
  });

  session_.SetUrl(url);
  session_.SetParameters(std::move(params));

  auto j = json::parse(session_.Get().text);
  return j.get<std::vector<JumpTypes::Asset>>();
}

JumpTypes::Asset PrivateJumpClient::get_asset(RequiredParameter id,
                                              OptionalParameter date) {
  auto url = build_url(cache_url_, "{}/asset/{}", HOST_URL, id);
  auto params = build_parameters({
      std::tuple{"date", date},
      // std::tuple{"fullResponse", full_response},
      std::tuple{"columns", OPT_STR("ASSET_DATABASE_ID")},
      std::tuple{"columns", OPT_STR("LABEL")},
      std::tuple{"columns", OPT_STR("LAST_CLOSE_VALUE_IN_CURR")},
      std::tuple{"columns", OPT_STR("TYPE")},
      std::tuple{"columns", OPT_STR("CURRENCY")},
  });

  session_.SetUrl(url);
  session_.SetParameters(std::move(params));

  auto j = json::parse(session_.Get().text);
  return j.get<JumpTypes::Asset>();
}

JumpTypes::JumpValue PrivateJumpClient::get_asset_attribute(
    RequiredParameter id, RequiredParameter attr_name, OptionalParameter date) {
  auto url = build_url(cache_url_, "{}/asset/{}/attribute/{}", HOST_URL, id,
                       attr_name);
  auto params = build_parameters({
      std::tuple{"date", date},
  });

  session_.SetUrl(url);
  session_.SetParameters(std::move(params));

  auto j = json::parse(session_.Get().text);
  return j.get<JumpTypes::JumpValue>();
}

std::vector<JumpTypes::Quote>
PrivateJumpClient::get_asset_quote(RequiredParameter id,
                                   OptionalParameter start_date,
                                   OptionalParameter end_date) {
  auto url = build_url(cache_url_, "{}/asset/{}/quote", HOST_URL, id);
  auto params = build_parameters({
      std::tuple{"start_date", start_date},
      std::tuple{"end_date", end_date},
  });

  session_.SetUrl(url);
  session_.SetParameters(std::move(params));

  auto j = json::parse(session_.Get().text);
  return j.get<std::vector<JumpTypes::Quote>>();
}

JumpTypes::Portfolio
PrivateJumpClient::get_portfolio_compo(RequiredParameter id) {
  auto url =
      build_url(cache_url_, "{}/portfolio/{}/dyn_amount_compo", HOST_URL, id);
  auto params = cpr::Parameters{};

  session_.SetUrl(url);
  session_.SetParameters(std::move(params));

  auto j = json::parse(session_.Get().text);
  return j.get<JumpTypes::Portfolio>();
}

void PrivateJumpClient::put_portfolio_compo(RequiredParameter id,
                                            JumpTypes::Portfolio &&portfolio) {
  auto url =
      build_url(cache_url_, "{}/portfolio/{}/dyn_amount_compo", HOST_URL, id);
  auto params = cpr::Parameters{};

  session_.SetUrl(url);
  session_.SetParameters(std::move(params));

  // Build and set the body
  json j_body = portfolio;
  session_.SetBody({j_body});

  // Send the PUT request
  session_.Put();

  // Remove the body from the session
  session_.SetBody({});
}

std::vector<JumpTypes::Ratio> PrivateJumpClient::get_ratios() {
  auto url = build_url(cache_url_, "{}/ratio", HOST_URL);
  auto params = cpr::Parameters{};

  session_.SetUrl(url);
  session_.SetParameters(std::move(params));

  auto j = json::parse(session_.Get().text);
  return j.get<std::vector<JumpTypes::Ratio>>();
}

JumpTypes::AssetRatioMap
PrivateJumpClient::compute_ratio(JumpTypes::RatioParam &&ratio_param) {
  auto url = build_url(cache_url_, "{}/ratio/invoke", HOST_URL);
  auto params = cpr::Parameters{};

  session_.SetUrl(url);
  session_.SetParameters(std::move(params));

  // Build and set the body
  json j_body = ratio_param;

  cpr::Body body = cpr::Body(std::move(j_body.dump()));
  session_.SetBody(body);

  // Send the request
  auto j = json::parse(session_.Post().text);

  // Clear the body
  session_.SetBody(cpr::Body());

  auto res = j.get<JumpTypes::AssetRatioMap>();

  return res;
}

double PrivateJumpClient::get_currency_change_rate(
    JumpTypes::CurrencyCode currency_src, JumpTypes::CurrencyCode currency_dest,
    OptionalParameter date) {
  auto url = build_url(cache_url_, "{}/currency/rate/{}/to/{}", HOST_URL,
                       currency_str(currency_src), currency_str(currency_dest));
  auto params = build_parameters({std::tuple{"date", date}});

  session_.SetUrl(url);
  session_.SetParameters(std::move(params));

  auto session_text = session_.Get().text;

  if (session_text.empty())
    return 1;

  auto j = json::parse(session_text);

  // The rate value is a "double number" that uses a comma instead of a dot
  // So we must do the parsing ourselves...
  auto rate_str = j.at("rate").at("value").get<std::string>();
  std::replace(rate_str.begin(), rate_str.end(), ',', '.');

  return std::stod(rate_str);
}
