/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: GPL-3.0-or-later
 *
 */
#pragma once

#include "coro/sync_wait.hpp"
#include "coro/task.hpp"

#include <core/domain/entities/Package.h>
#include <core/domain/repositories/RepositoryBase.h>
#include <functional>
#include <span>
#include <vector>

namespace bxt::Core::Domain {
struct PackageRepositoryBase : public ReadWriteRepositoryBase<Package> {
    template<typename T>
    using ReadResult = ReadOnlyRepositoryBase<Package>::Result<T>;
    template<typename T>
    using WriteResult = ReadWriteRepositoryBase<Package>::Result<T>;

    virtual coro::task<TResults>
        find_by_section_async(const Section section) const = 0;

    virtual TResults find_by_section(const Section& section) {
        return coro::sync_wait(find_by_section_async(section));
    }

    virtual coro::task<TResults> find_by_section_async(
        const Section section,
        const std::function<bool(const Package& pkg)> predicate) const = 0;

    virtual TResults find_by_section(
        const Section section,
        const std::function<bool(const Package& pkg)> predicate) const {
        return coro::sync_wait(find_by_section_async(section, predicate));
    }
};
} // namespace bxt::Core::Domain
