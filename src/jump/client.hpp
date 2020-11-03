#pragma once

#include "types.hpp"

#include <string>

#include <cpr/cpr.h>
#include <fmt/format.h>
#include <optional>
#include <tuple>
#include <vector>

using json = nlohmann::json;

class JumpClient {
public:
  using ParameterName = std::string &&;
  using RequiredParameter = std::string &&;
  using OptionalParameter = std::optional<std::string> &&;

  JumpClient(std::string &&username, std::string &&password);

  static cpr::Parameters build_parameters(
      std::initializer_list<std::tuple<ParameterName, OptionalParameter>>
          &&opt_params);

  /** GET /asset
   * Récupération des informations d'actifs disponibles pour la sélection
   * d'actif
   *
   * \param date Date de valeur au format RFC 3339
   * (https://www.ietf.org/rfc/rfc3339.txt) pour la récupération des données
   * (par défaut : date du jour)
   * \param full_response Si vrai, la réponse envoyée contient l'intégralité des
   * informations disponibles du point d'entrée, sinon ne renvoie que le
   * sous-ensemble d'informations ayant une valeur
   * \param columns Paramètre de selection des colonnes. Si des valeurs est
   * envoyées, seul les colonnes dont le nom est inclus seront retournés
   */
  std::vector<JumpTypes::asset> get_assets(OptionalParameter date,
                                           OptionalParameter full_response,
                                           OptionalParameter columns);

  /** GET /ratio
   * Récuperation de la liste des ratios disponibles
   */
  std::vector<JumpTypes::ratio> get_ratios();

private:
  constexpr static std::string_view HOST_URL =
      "https://dolphin.jump-technology.com:8443/api/v1";

  /** The session to the API server, makes it able to reuse parameters */
  cpr::Session session_;

  /** String cache to build the urls */
  fmt::memory_buffer cache_url_;
};
