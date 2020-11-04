#pragma once

#include "client.hpp"

#include <cpr/session.h>
#include <fmt/format.h>

class PrivateJumpClient final : public JumpClient {
public:
  using ParameterName = std::string &&;
  using RequiredParameter = std::string &&;
  using OptionalParameter = std::optional<std::string> &&;

  PrivateJumpClient(std::string &&username, std::string &&password);

  std::vector<JumpTypes::asset>
  get_assets(OptionalParameter date = std::nullopt,
             OptionalParameter full_response = std::nullopt,
             OptionalParameter columns = std::nullopt) override;

  JumpTypes::asset get_asset(RequiredParameter id,
                             OptionalParameter date = std::nullopt,
                             OptionalParameter full_response = std::nullopt,
                             OptionalParameter columns = std::nullopt) override;

  JumpTypes::jump_value_string
  get_asset_attribute(RequiredParameter id, RequiredParameter attr_name,
                      OptionalParameter date = std::nullopt,
                      OptionalParameter full_response = std::nullopt) override;

  std::vector<JumpTypes::quote>
  get_asset_quote(RequiredParameter id,
                  OptionalParameter start_date = std::nullopt,
                  OptionalParameter end_date = std::nullopt) override;

  JumpTypes::portfolio get_portfolio_compo(RequiredParameter id) override;

  void put_portfolio_compo(RequiredParameter id,
                           JumpTypes::portfolio &&portfolio) override;

  std::vector<JumpTypes::ratio> get_ratios() override;

  JumpTypes::asset_ratio_map
  compute_ratio(JumpTypes::ratio_param &&ratio_param,
                OptionalParameter full_response = std::nullopt) override;

private:
  constexpr static const char *HOST_URL =
      "https://dolphin.jump-technology.com:8443/api/v1";

  /** The session to the API server, makes it able to reuse parameters */
  cpr::Session session_;

  /** String cache to build the urls */
  fmt::memory_buffer cache_url_;
};
