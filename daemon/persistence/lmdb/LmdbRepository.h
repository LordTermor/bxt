/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: GPL-3.0-or-later
 *
 */
#pragma once

#include "core/application/dtos/UserDTO.h"
#include "core/domain/entities/PackageLogEntry.h"
#include "core/domain/entities/PackageUpdateLogEntry.h"
#include "core/domain/repositories/RepositoryBase.h"
#include "core/domain/repositories/UnitOfWorkBase.h"
#include "coro/task.hpp"
#include "coro/when_all.hpp"
#include "nonstd/expected.hpp"
#include "utilities/Error.h"
#include "utilities/StaticDTOMapper.h"
#include "utilities/errors/DatabaseError.h"
#include "utilities/lmdb/Database.h"
#include "utilities/lmdb/Environment.h"

#include <coro/coro.hpp>
#include <vector>

namespace bxt::Persistence {

template<typename TEntity, typename TDTO>
class LmdbRepositoryBase
    : public bxt::Core::Domain::ReadWriteRepositoryBase<TEntity> {
public:
    LmdbRepositoryBase(
        std::shared_ptr<bxt::Utilities::LMDB::Environment> environment,
        const std::string &database_name)
        : m_environment(environment), m_db(m_environment, database_name) {}

    using TResult =
        typename bxt::Core::Domain::ReadWriteRepositoryBase<TEntity>::TResult;
    using TResults =
        typename bxt::Core::Domain::ReadWriteRepositoryBase<TEntity>::TResults;
    using TId =
        typename bxt::Core::Domain::ReadWriteRepositoryBase<TEntity>::TId;

    using TEntities =
        typename bxt::Core::Domain::ReadWriteRepositoryBase<TEntity>::TEntities;

    template<typename T>
    using WriteResult = typename bxt::Core::Domain::ReadWriteRepositoryBase<
        TEntity>::template Result<T>;

    using TMapper = Utilities::StaticDTOMapper<TEntity, TDTO>;

    virtual coro::task<TResult> find_by_id_async(TId id) override {
        using namespace bxt::Core::Domain;

        const auto entity = co_await m_db.get(std::string(id));

        if (!entity.has_value()) {
            co_return bxt::make_error<ReadError>(
                ReadError::ErrorTypes::EntityNotFound);
        }

        co_return TMapper::to_entity(*entity);
    }
    virtual coro::task<TResult>
        find_first_async(std::function<bool(const TEntity &)>) override {}
    virtual coro::task<TResults>
        find_async(std::function<bool(const TEntity &)> condition) override {}
    virtual coro::task<TResults> all_async() override {
        TEntities results;

        auto rotxn =
            lmdb::txn::begin(m_environment->env(), nullptr, MDB_RDONLY);

        {
            auto cursor = lmdb::cursor::open(rotxn, m_db.dbi());

            std::string_view key, value;
            if (cursor.get(key, value, MDB_FIRST)) {
                do {
                    auto res =
                        Utilities::LMDB::BoostSerializer<TDTO>::deserialize(
                            value);

                    if (!res.has_value()) {
                        co_return bxt::make_error_with_source<ReadError>(
                            std::move(res.error()),
                            ReadError::ErrorTypes::EntityFindError);
                    }

                    results.emplace_back(TMapper::to_entity(*res));
                } while (cursor.get(key, value, MDB_NEXT));
            }
        }

        co_return results;
    }

    virtual coro::task<WriteResult<void>>
        add_async(const TEntity entity) override {
        m_to_add.push_back(entity);
        co_return {};
    }
    virtual coro::task<WriteResult<void>>
        update_async(const TEntity entity) override {
        m_to_update.push_back(entity);
        co_return {};
    }
    virtual coro::task<WriteResult<void>> remove_async(const TId id) override {
        m_to_remove.push_back(id);
    }

    virtual coro::task<UnitOfWorkBase::Result<void>> commit_async() override {
        std::vector<coro::task<typename decltype(m_db)::template Result<bool>>>
            tasks;

        for (const auto &el : m_to_add) {
            tasks.push_back(
                m_db.put(std::string(el.id()), TMapper::to_dto(el)));
        }
        for (const auto &el : m_to_remove) {
            tasks.push_back(m_db.del(std::string(el)));
        }

        co_await coro::when_all(std::move(tasks));

        co_return {};
    }
    virtual coro::task<UnitOfWorkBase::Result<void>> rollback_async() override {
        m_to_add.clear();
        m_to_remove.clear();
        m_to_update.clear();

        co_return {};
    }

    virtual std::vector<Core::Domain::Events::EventPtr>
        event_store() const override {
        return m_event_store;
    };

private:
    std::shared_ptr<bxt::Utilities::LMDB::Environment> m_environment;
    Utilities::LMDB::Database<TDTO> m_db;

    std::vector<TEntity> m_to_add;
    std::vector<TId> m_to_remove;
    std::vector<TEntity> m_to_update;

    std::vector<Core::Domain::Events::EventPtr> m_event_store;
};

} // namespace bxt::Persistence
