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

  std::vector<JumpTypes::Asset>
  get_assets(OptionalParameter date = std::nullopt,
             OptionalParameter full_response = std::nullopt,
             OptionalParameter columns = std::nullopt) override;

  JumpTypes::Asset get_asset(RequiredParameter id,
                             OptionalParameter date = std::nullopt,
                             OptionalParameter full_response = std::nullopt,
                             OptionalParameter columns = std::nullopt) override;

  JumpTypes::JumpValueString
  get_asset_attribute(RequiredParameter id, RequiredParameter attr_name,
                      OptionalParameter date = std::nullopt,
                      OptionalParameter full_response = std::nullopt) override;

  std::vector<JumpTypes::Quote>
  get_asset_quote(RequiredParameter id,
                  OptionalParameter start_date = std::nullopt,
                  OptionalParameter end_date = std::nullopt) override;

  JumpTypes::Portfolio get_portfolio_compo(RequiredParameter id) override;

  void put_portfolio_compo(RequiredParameter id,
                           JumpTypes::Portfolio &&portfolio) override;

  std::vector<JumpTypes::Ratio> get_ratios() override;

  JumpTypes::AssetRatioMap
  compute_ratio(JumpTypes::RatioParam &&ratio_param,
                OptionalParameter full_response = std::nullopt) override;

  double
  get_currency_change_rate(JumpTypes::CurrencyCode currency_src,
                           JumpTypes::CurrencyCode currency_dest,
                           OptionalParameter date = std::nullopt) override;

private:
  constexpr static std::string_view HOST_URL =
      "https://dolphin.jump-technology.com:8443/api/v1";

  /** The session to the API server, makes it able to reuse parameters */
  cpr::Session session_;

  /** String cache to build the urls */
  fmt::memory_buffer cache_url_;
};
