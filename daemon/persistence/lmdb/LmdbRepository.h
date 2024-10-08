/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#pragma once

#include "core/application/dtos/UserDTO.h"
#include "core/domain/repositories/RepositoryBase.h"
#include "core/domain/repositories/UnitOfWorkBase.h"
#include "coro/task.hpp"
#include "coro/when_all.hpp"
#include "persistence/lmdb/LmdbUnitOfWork.h"
#include "utilities/Error.h"
#include "utilities/errors/DatabaseError.h"
#include "utilities/lmdb/Database.h"
#include "utilities/lmdb/Environment.h"
#include "utilities/StaticDTOMapper.h"

#include <coro/coro.hpp>
#include <memory>
#include <optional>
#include <vector>

namespace bxt::Persistence {

template<typename TEntity, typename TDTO>
class LmdbRepositoryBase : public bxt::Core::Domain::ReadWriteRepositoryBase<TEntity> {
public:
    LmdbRepositoryBase(std::shared_ptr<bxt::Utilities::LMDB::Environment> environment,
                       std::string const& database_name)
        : m_environment(environment)
        , m_db(m_environment, database_name) {
    }

    using TResult = typename bxt::Core::Domain::ReadWriteRepositoryBase<TEntity>::TResult;
    using TResults = typename bxt::Core::Domain::ReadWriteRepositoryBase<TEntity>::TResults;
    using TId = typename bxt::Core::Domain::ReadWriteRepositoryBase<TEntity>::TId;

    using TEntities = typename bxt::Core::Domain::ReadWriteRepositoryBase<TEntity>::TEntities;

    template<typename T>
    using WriteResult =
        typename bxt::Core::Domain::ReadWriteRepositoryBase<TEntity>::template Result<T>;

    using ReadError = typename bxt::Core::Domain::ReadError;
    using WriteError = typename bxt::Core::Domain::WriteError;

    using TMapper = Utilities::StaticDTOMapper<TEntity, TDTO>;

    coro::task<TResult>
        find_by_id_async(TId id, std::shared_ptr<Core::Domain::UnitOfWorkBase> uow) override {
        using namespace bxt::Core::Domain;

        auto lmdb_uow = std::dynamic_pointer_cast<LmdbUnitOfWork>(uow);
        if (!lmdb_uow) {
            co_return bxt::make_error<ReadError>(ReadError::InvalidArgument);
        }

        auto const entity = co_await m_db.get(lmdb_uow->txn().value, std::string(id));

        if (!entity.has_value()) {
            co_return bxt::make_error<ReadError>(ReadError::EntityNotFound);
        }

        co_return TMapper::to_entity(*entity);
    }
    coro::task<TResult>
        find_first_async(std::function<bool(TEntity const&)> condition,
                         std::shared_ptr<Core::Domain::UnitOfWorkBase> uow) override {
        auto lmdb_uow = std::dynamic_pointer_cast<LmdbUnitOfWork>(uow);
        if (!lmdb_uow) {
            co_return bxt::make_error<ReadError>(ReadError::InvalidArgument);
        }

        std::optional<TResult> result;

        co_await m_db.accept(lmdb_uow->txn().value,
                             [&result, &condition](std::string_view key, const TDTO& e) {
                                 TEntity entity = TMapper::to_entity(e);
                                 if (condition(entity)) {
                                     result = entity;
                                     return Utilities::NavigationAction::Stop;
                                 }
                                 return Utilities::NavigationAction::Next;
                             });

        if (!result.has_value()) {
            co_return bxt::make_error<ReadError>(ReadError::EntityNotFound);
        }

        co_return *result;
    }

    coro::task<TResults> find_async(std::function<bool(TEntity const&)> condition,
                                    std::shared_ptr<Core::Domain::UnitOfWorkBase> uow) override {
        TResults results = {};
        auto lmdb_uow = std::dynamic_pointer_cast<LmdbUnitOfWork>(uow);
        if (!lmdb_uow) {
            co_return bxt::make_error<ReadError>(ReadError::InvalidArgument);
        }

        co_await m_db.accept(lmdb_uow->txn().value,
                             [&results, &condition](std::string_view key, const TDTO& e) {
                                 TEntity entity = TMapper::to_entity(e);
                                 if (condition(entity)) {
                                     results->push_back(entity);
                                 }
                                 return Utilities::NavigationAction::Next;
                             });

        co_return results;
    }

    coro::task<TResults> all_async(std::shared_ptr<Core::Domain::UnitOfWorkBase> uow) override {
        TEntities results;
        auto lmdb_uow = std::dynamic_pointer_cast<LmdbUnitOfWork>(uow);
        if (!lmdb_uow) {
            co_return bxt::make_error<ReadError>(ReadError::InvalidArgument);
        }
        co_await m_db.accept(lmdb_uow->txn().value,
                             [&results]([[maybe_unused]] std::string_view key, const TDTO& e) {
                                 results.push_back(TMapper::to_entity(e));
                                 return Utilities::NavigationAction::Next;
                             });

        co_return results;
    }
    coro::task<WriteResult<void>>
        add_async(TEntity const entity,
                  std::shared_ptr<Core::Domain::UnitOfWorkBase> uow) override {
        auto lmdb_uow = std::dynamic_pointer_cast<LmdbUnitOfWork>(uow);
        if (!lmdb_uow) {
            co_return bxt::make_error<WriteError>(WriteError::InvalidArgument);
        }

        auto result = co_await m_db.put(lmdb_uow->txn().value, std::string(entity.id()),
                                        TMapper::to_dto(entity));
        if (!result) {
            co_return bxt::make_error_with_source<WriteError>(std::move(result.error()),
                                                              WriteError::OperationError);
        }

        co_return {};
    }

    coro::task<WriteResult<void>>
        update_async(TEntity const entity,
                     std::shared_ptr<Core::Domain::UnitOfWorkBase> uow) override {
        auto lmdb_uow = std::dynamic_pointer_cast<LmdbUnitOfWork>(uow);
        if (!lmdb_uow) {
            co_return bxt::make_error<WriteError>(WriteError::InvalidArgument);
        }

        auto result = co_await m_db.put(lmdb_uow->txn().value, std::string(entity.id()),
                                        TMapper::to_dto(entity));
        if (!result) {
            co_return bxt::make_error_with_source<WriteError>(std::move(result.error()),
                                                              WriteError::OperationError);
        }

        co_return {};
    }

    coro::task<WriteResult<void>>
        delete_async(TId const id, std::shared_ptr<Core::Domain::UnitOfWorkBase> uow) override {
        auto lmdb_uow = std::dynamic_pointer_cast<LmdbUnitOfWork>(uow);
        if (!lmdb_uow) {
            co_return bxt::make_error<WriteError>(WriteError::InvalidArgument);
        }

        auto result = co_await m_db.del(lmdb_uow->txn().value, std::string(id));
        if (!result) {
            co_return bxt::make_error_with_source<WriteError>(std::move(result.error()),
                                                              WriteError::OperationError);
        }

        co_return {};
    }

private:
    std::shared_ptr<bxt::Utilities::LMDB::Environment> m_environment;
    Utilities::LMDB::Database<TDTO> m_db;
};

} // namespace bxt::Persistence
