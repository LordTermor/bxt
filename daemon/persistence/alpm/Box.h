/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: GPL-3.0-or-later
 *
 */
#pragma once

#include "core/application/dtos/PackageSectionDTO.h"
#include "core/domain/entities/Package.h"
#include "core/domain/repositories/PackageRepositoryBase.h"
#include "core/domain/repositories/RepositoryBase.h"
#include "core/domain/repositories/UnitOfWorkBase.h"
#include "persistence/alpm/BoxOptions.h"
#include "utilities/alpmdb/Database.h"

#include <coro/sync_wait.hpp>
#include <functional>

namespace bxt::Persistence {

class Box : public Core::Domain::PackageRepositoryBase {
public:
    Box(ReadOnlyRepositoryBase<Section> &section_repository)
        : m_section_repository(section_repository) {
        auto sections = coro::sync_wait(m_section_repository.all_async());
        if (!sections.has_value()) { return; }

        for (const auto &section : *sections) {
            auto dto = SectionDTOMapper::to_dto(section);

            auto path_for_section =
                fmt::format("{}/{}", m_options.location, std::string(dto));

            std::filesystem::create_directories(path_for_section);

            m_map.emplace(dto, Utilities::AlpmDb::Database {path_for_section,
                                                            dto.repository});
        }
    };

    virtual coro::task<TResult> find_by_id_async(TId id) override;
    virtual coro::task<TResult>
        find_first_async(std::function<bool(const Package &)>) override;
    virtual coro::task<TResults>
        find_async(std::function<bool(const Package &)> condition) override;
    virtual coro::task<TResults> all_async() override;

    virtual coro::task<WriteResult<void>>
        add_async(const Package entity) override;
    virtual coro::task<WriteResult<void>>
        add_async(const std::vector<Package> entity) override;

    virtual coro::task<WriteResult<void>>
        update_async(const Package entity) override;
    virtual coro::task<WriteResult<void>> remove_async(const TId id) override;

    virtual coro::task<TResults>
        find_by_section_async(const Section section) const override;

    virtual coro::task<TResults> find_by_section_async(
        const Section section,
        const std::function<bool(const Package &)> predicate) const override;

    virtual coro::task<UnitOfWorkBase::Result<void>> commit_async() override;
    virtual coro::task<UnitOfWorkBase::Result<void>> rollback_async() override;

    virtual std::vector<Events::EventPtr> event_store() const override;

private:
    BoxOptions m_options;
    ReadOnlyRepositoryBase<Section> &m_section_repository;

    phmap::parallel_flat_hash_map<Core::Application::PackageSectionDTO,
                                  Utilities::AlpmDb::Database>
        m_map;

    std::filesystem::path m_root_path;

    std::vector<Package> m_to_add;
    std::vector<TId> m_to_remove;
    std::vector<Package> m_to_update;

    std::vector<Events::EventPtr> m_event_store;
};

} // namespace bxt::Persistence
