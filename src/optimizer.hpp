#pragma once

#include "jump/client.hpp"
#include "tree.hpp"

/** Try to a composition by changing the number of shares
 * The resulting compo
 */
compo_t optimize_compo_stochastic(const TrucsInteressants &trucs,
                                  compo_t compo);
