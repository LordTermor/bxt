/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */

#pragma once

#include "utilities/configuration/Configuration.h"

#include <string>
namespace bxt::Presentation {

struct JwtOptions {
    std::string secret = "secret";

    void serialize(Utilities::Configuration &config) {
        config.set("jwt-secret", secret);
    }
    void deserialize(const Utilities::Configuration &config) {
        secret = config.get<std::string>("jwt-secret").value_or(secret);
    }
};
} // namespace bxt::Presentation
