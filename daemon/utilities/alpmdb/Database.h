/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2022 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: GPL-3.0-or-later
 *
 */
#pragma once
#include "utilities/alpmdb/Desc.h"

#include <coro/event.hpp>
#include <coro/io_scheduler.hpp>
#include <coro/mutex.hpp>
#include <coro/sync_wait.hpp>
#include <coro/task.hpp>
#include <filesystem>
#include <frozen/set.h>
#include <frozen/string.h>
#include <parallel_hashmap/phmap.h>

namespace bxt::Utilities::AlpmDb {

struct DatabaseParseException : public std::runtime_error {
    explicit DatabaseParseException(const std::string& what)
        : std::runtime_error(what) {}
};

class Database {
public:
    explicit Database(const std::filesystem::path& path = "./",
                      const std::string& name = "")
        : m_path(std::filesystem::absolute(path)), m_name(name) {
        if (name == "") { m_name = m_path.parent_path().filename(); }

        coro::sync_wait(load());
    }

    coro::task<void> add(const std::set<std::string>& files);
    coro::task<void> remove(const std::set<std::string>& packages);
    std::set<std::string> packages() const {
        std::set<std::string> packages;
        std::ranges::transform(m_descriptions,
                               std::inserter(packages, packages.begin()),
                               [](const auto& value) { return value.first; });

        return packages;
    }

    coro::task<void> load();

private:

    coro::task<void> save_async();

    bool package_exists(const std::string& package_name) const;
    void create_symlinks() const;

    constexpr static frozen::set<frozen::string, 3>
        supported_package_extensions = {"pkg.tar.gz", "pkg.tar.xz",
                                        "pkg.tar.zst"};

    std::filesystem::path m_path;
    std::unique_ptr<coro::mutex> m_file_lock = std::make_unique<coro::mutex>();

    std::string m_name;
    phmap::parallel_node_hash_map<std::string, Desc> m_descriptions;
};
} // namespace bxt::Utilities::AlpmDb
