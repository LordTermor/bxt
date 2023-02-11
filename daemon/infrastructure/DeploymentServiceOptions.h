/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: GPL-3.0-or-later
 *
 */
#pragma once

#include "core/application/dtos/PackageSectionDTO.h"
#include "utilities/repo-schema/SchemaExtension.h"

#include <parallel_hashmap/phmap.h>

namespace bxt::Infrastructure {

class DeploymentServiceOptions : public Utilities::RepoSchema::Extension {
public:
    std::filesystem::path
        pool(const Core::Application::PackageSectionDTO& section) const;

    virtual void parse(const YAML::Node& root_node) override;

    std::string token() const { return m_token; }

private:
    phmap::flat_hash_map<Core::Application::PackageSectionDTO,
                         std::filesystem::path>
        m_pool_path_overrides;

    std::filesystem::path m_default_pool_path = "box/pool/overlay/";
    std::string m_token = "CHANGEME";
};

} // namespace bxt::Infrastructure