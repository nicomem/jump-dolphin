#pragma once

#include "types.hpp"

#include <string>

#include <cpr/session.h>
#include <fmt/format.h>
#include <optional>
#include <vector>

class JumpClient {
public:
  using ParameterName = std::string &&;
  using RequiredParameter = std::string &&;
  using OptionalParameter = std::optional<std::string> &&;

  JumpClient(std::string &&username, std::string &&password);

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
  std::vector<JumpTypes::asset>
  get_assets(OptionalParameter date = std::nullopt,
             OptionalParameter full_response = std::nullopt,
             OptionalParameter columns = std::nullopt);

  /** GET /asset/{id}
   * Récupération des informations d'un actif spécifique, déterminé par son
   * identifiant technique
   *
   * \param id Identifiant technique de l'actif, représenté par l'information
   * "ASSET_DATABASE_ID"
   * \param date Date de valeur au format RFC 3339
   * (https://www.ietf.org/rfc/rfc3339.txt) pour la récupération des données
   * (par défaut : date du jour)
   * \param full_response Si vrai, la réponse envoyée contient l'intégralité des
   * informations disponibles du point d'entrée, sinon ne renvoie que le
   * sous-ensemble d'informations ayant une valeur
   * \param columns Paramètre de selection des colonnes. Si des valeurs est
   * envoyées, seul les colonnes dont le nom est inclus seront retournés
   */
  JumpTypes::asset get_asset(RequiredParameter id,
                             OptionalParameter date = std::nullopt,
                             OptionalParameter full_response = std::nullopt,
                             OptionalParameter columns = std::nullopt);

  /** GET /asset/{id}/attribute/{attr_name}
   * Récupère une information spécifique d'un actif déterminé par son
   * identifiant technique
   *
   * \param id Identifiant technique de l'actif, représenté par l'information
   * "ASSET_DATABASE_ID"
   * \param attr_name Nom de l'attribut à récupérer, voir type 'asset_info_list'
   * pour la liste des informations disponibles
   * \param date Date de valeur au format RFC 3339
   * (https://www.ietf.org/rfc/rfc3339.txt) pour la récupération des données
   * (par défaut : date du jour)
   * \param full_response Si vrai, la réponse envoyée contient l'intégralité des
   * informations disponibles du point d'entrée, sinon ne renvoie que le
   * sous-ensemble d'informations ayant une valeur
   */
  JumpTypes::jump_value_string
  get_asset_attribute(RequiredParameter id, RequiredParameter attr_name,
                      OptionalParameter date = std::nullopt,
                      OptionalParameter full_response = std::nullopt);

  /** GET /asset/{id}/quote
   * Récupèration des cotations d'un actif triées par date
   *
   * \param id Identifiant technique de l'actif, représenté par l'information
   * "ASSET_DATABASE_ID"
   * \param start_date Date de début de la période souhaitée au format RFC 3339
   * (https://www.ietf.org/rfc/rfc3339.txt) pour la récupération des données
   * (par défaut : 1ere date)
   * \param end_date Date de fin de la période souhaitée au format RFC 3339
   * (https://www.ietf.org/rfc/rfc3339.txt) pour la récupération des données
   * (par défaut : date du jour)
   */
  std::vector<JumpTypes::quote>
  get_asset_quote(RequiredParameter id,
                  OptionalParameter start_date = std::nullopt,
                  OptionalParameter end_date = std::nullopt);

  /** GET /portfolio/{id}/dyn_amount_compo
   * Récupération d'un portefeuille de composition historique
   *
   * \param id Identifiant technique du portefeuille, représenté par
   * l'information "ASSET_DATABASE_ID"
   */
  JumpTypes::portfolio get_portfolio_compo(RequiredParameter id);

  /** PUT /portfolio/{id}/dyn_amount_compo
   * Mise a jour d'un portefeuille de composition historique
   *
   * \param id Identifiant technique du portefeuille, représenté par
   * l'information "ASSET_DATABASE_ID"
   * \param portfolio The portfolio to send
   */
  void put_portfolio_compo(RequiredParameter id,
                           JumpTypes::portfolio &&portfolio);

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
