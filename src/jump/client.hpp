#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace JumpTypes {
class asset_ratio_map;
class asset;
class jump_value_string;
class portfolio;
class quote;
class ratio_param;
class ratio;
} // namespace JumpTypes

struct JumpClient {
  using ParameterName = std::string &&;
  using RequiredParameter = std::string &&;
  using OptionalParameter = std::optional<std::string> &&;

  /** Initialize a new JUMP client */
  static std::unique_ptr<JumpClient> build(std::string &&username,
                                           std::string &&password);

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
  virtual std::vector<JumpTypes::asset>
  get_assets(OptionalParameter date = std::nullopt,
             OptionalParameter full_response = std::nullopt,
             OptionalParameter columns = std::nullopt) = 0;

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
  virtual JumpTypes::asset
  get_asset(RequiredParameter id, OptionalParameter date = std::nullopt,
            OptionalParameter full_response = std::nullopt,
            OptionalParameter columns = std::nullopt) = 0;

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
  virtual JumpTypes::jump_value_string
  get_asset_attribute(RequiredParameter id, RequiredParameter attr_name,
                      OptionalParameter date = std::nullopt,
                      OptionalParameter full_response = std::nullopt) = 0;

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
  virtual std::vector<JumpTypes::quote>
  get_asset_quote(RequiredParameter id,
                  OptionalParameter start_date = std::nullopt,
                  OptionalParameter end_date = std::nullopt) = 0;

  /** GET /portfolio/{id}/dyn_amount_compo
   * Récupération d'un portefeuille de composition historique
   *
   * \param id Identifiant technique du portefeuille, représenté par
   * l'information "ASSET_DATABASE_ID"
   */
  virtual JumpTypes::portfolio get_portfolio_compo(RequiredParameter id) = 0;

  /** PUT /portfolio/{id}/dyn_amount_compo
   * Mise a jour d'un portefeuille de composition historique
   *
   * \param id Identifiant technique du portefeuille, représenté par
   * l'information "ASSET_DATABASE_ID"
   * \param portfolio The portfolio to send
   */
  virtual void put_portfolio_compo(RequiredParameter id,
                                   JumpTypes::portfolio &&portfolio) = 0;

  /** GET /ratio
   * Récuperation de la liste des ratios disponibles
   */
  virtual std::vector<JumpTypes::ratio> get_ratios() = 0;

  /** POST /ratio/invoke
   * Calcul du résultat d'une liste de ratios sur une liste d'actifs
   *
   * \param ratio_param Paramètre d'exécution contenant une liste d'actifs, une
   * liste de ratios, une période, une fréquence ainsi qu'un actif benchmark
   * \param full_response Si vrai, la réponse envoyée contient l'intégralité des
   * informations disponibles du point d'entrée, sinon ne renvoie que le
   * sous-ensemble d'informations ayant une valeur
   */
  virtual JumpTypes::asset_ratio_map
  compute_ratio(JumpTypes::ratio_param &&ratio_param,
                OptionalParameter full_response = std::nullopt) = 0;
};
