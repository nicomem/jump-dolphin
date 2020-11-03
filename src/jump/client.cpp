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
JumpClient::get_assets(JumpClient::OptionalParameter date,
                       JumpClient::OptionalParameter full_response,
                       JumpClient::OptionalParameter columns) {
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

std::vector<JumpTypes::ratio> JumpClient::get_ratios() {
  auto url = build_url(cache_url_, "{}/ratio", HOST_URL);
  auto params = cpr::Parameters{};

  session_.SetUrl(url);
  session_.SetParameters(std::move(params));

  auto j = json::parse(session_.Get().text);
  return j.get<std::vector<JumpTypes::ratio>>();
}
