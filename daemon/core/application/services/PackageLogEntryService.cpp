/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: %YEAR% Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: GPL-3.0-or-later
 *
 */
#include "PackageLogEntryService.h"

#include "core/domain/events/PackageEvents.h"

namespace bxt::Core::Application {

void PackageLogEntryService::init() {
    m_listener.listen<bxt::Core::Domain::Events::PackageAdded>(
        [this](const Domain::Events::PackageAdded &added) {
            Domain::PackageLogEntry entry(added.package,
                                          Domain::LogEntryType::Add);

            m_repository.add(entry);
            coro::sync_wait(m_repository.commit_async());
        });
    //    listener.listen<bxt::Core::Domain::Events::PackageRemoved>(
    //        [this](const Domain::Events::PackageRemoved &added) {
    //            Domain::PackageLogEntry entry(added.id,
    //            Domain::LogEntryType::Add);

    //            m_repository.add(entry);
    //        });
    m_listener.listen<bxt::Core::Domain::Events::PackageUpdated>(
        [this](const Domain::Events::PackageUpdated &added) {
            Domain::PackageLogEntry entry(added.new_package,
                                          Domain::LogEntryType::Add);

            m_repository.add(entry);
            coro::sync_wait(m_repository.commit_async());
        });
}

coro::task<std::vector<PackageLogEntryDTO>> PackageLogEntryService::events() {
    std::vector<PackageLogEntryDTO> result;

    auto entities = co_await m_repository.all_async();

    std::ranges::transform(
        entities, std::back_inserter(result),
        Utilities::StaticDTOMapper<PackageLogEntry,
                                   PackageLogEntryDTO>::to_dto);

    co_return result;
}

} // namespace bxt::Core::Application
