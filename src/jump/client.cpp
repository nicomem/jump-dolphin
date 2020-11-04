#include "client.hpp"

#include <tuple>
#include <vector>

using ParamOptKV =
    std::tuple<JumpClient::ParameterName, JumpClient::OptionalParameter>;

static cpr::Parameters
build_parameters(std::initializer_list<ParamOptKV> &&opt_params) {
  auto holder = cpr::CurlHolder();
  auto params = cpr::Parameters();

  for (auto &&[key, value] : opt_params) {
    // Only add the parameter if the value is present
    if (value.has_value()) {
      // Create the parameter
      auto param = cpr::Parameter{key, value.value()};

      // And add it to the parameters
      params.AddParameter(param, holder);
    }
  }

  return params;
}

JumpClient::JumpClient(std::string &&username, std::string &&password)
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

std::vector<JumpTypes::asset>
JumpClient::get_assets(OptionalParameter date, OptionalParameter full_response,
                       OptionalParameter columns) {
  auto url = build_url(cache_url_, "{}/asset", HOST_URL);
  auto params = build_parameters({
      std::tuple{"date", date},
      std::tuple{"fullResponse", full_response},
      std::tuple{"columns", columns},
  });

  session_.SetUrl(url);
  session_.SetParameters(std::move(params));

  auto j = json::parse(session_.Get().text);
  return j.get<std::vector<JumpTypes::asset>>();
}

JumpTypes::asset JumpClient::get_asset(RequiredParameter id,
                                       OptionalParameter date,
                                       OptionalParameter full_response,
                                       OptionalParameter columns) {
  auto url = build_url(cache_url_, "{}/asset/{}", HOST_URL, id);
  auto params = build_parameters({
      std::tuple{"date", date},
      std::tuple{"fullResponse", full_response},
      std::tuple{"columns", columns},
  });

  session_.SetUrl(url);
  session_.SetParameters(std::move(params));

  auto j = json::parse(session_.Get().text);
  return j.get<JumpTypes::asset>();
}

JumpTypes::jump_value_string JumpClient::get_asset_attribute(
    RequiredParameter id, RequiredParameter attr_name, OptionalParameter date,
    OptionalParameter full_response) {
  auto url = build_url(cache_url_, "{}/asset/{}/attribute/{}", HOST_URL, id,
                       attr_name);
  auto params = build_parameters({
      std::tuple{"date", date},
      std::tuple{"fullResponse", full_response},
  });

  session_.SetUrl(url);
  session_.SetParameters(std::move(params));

  auto j = json::parse(session_.Get().text);
  return j.get<JumpTypes::jump_value_string>();
}

std::vector<JumpTypes::quote>
JumpClient::get_asset_quote(RequiredParameter id, OptionalParameter start_date,
                            OptionalParameter end_date) {
  auto url = build_url(cache_url_, "{}/asset/{}/quote", HOST_URL, id);
  auto params = build_parameters({
      std::tuple{"start_date", start_date},
      std::tuple{"end_date", end_date},
  });

  session_.SetUrl(url);
  session_.SetParameters(std::move(params));

  auto j = json::parse(session_.Get().text);
  return j.get<std::vector<JumpTypes::quote>>();
}

JumpTypes::portfolio JumpClient::get_portfolio_compo(RequiredParameter id) {
  auto url =
      build_url(cache_url_, "{}/portfolio/{}/dyn_amount_compo", HOST_URL, id);
  auto params = cpr::Parameters{};

  session_.SetUrl(url);
  session_.SetParameters(std::move(params));

  auto j = json::parse(session_.Get().text);
  return j.get<JumpTypes::portfolio>();
}

void JumpClient::put_portfolio_compo(RequiredParameter id,
                                     JumpTypes::portfolio &&portfolio) {
  auto url =
      build_url(cache_url_, "{}/portfolio/{}/dyn_amount_compo", HOST_URL, id);
  auto params = cpr::Parameters{};

  session_.SetUrl(url);
  session_.SetParameters(std::move(params));

  // Build and set the body
  json j_body = portfolio;
  auto body = cpr::Body{j_body.dump()};
  session_.SetBody(std::move(body));

  // Send the PUT request
  session_.Put();

  // Remove the body from the session
  session_.SetBody({});
}

std::vector<JumpTypes::ratio> JumpClient::get_ratios() {
  auto url = build_url(cache_url_, "{}/ratio", HOST_URL);
  auto params = cpr::Parameters{};

  session_.SetUrl(url);
  session_.SetParameters(std::move(params));

  auto j = json::parse(session_.Get().text);
  return j.get<std::vector<JumpTypes::ratio>>();
}
