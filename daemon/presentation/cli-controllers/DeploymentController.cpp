/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
#include "DeploymentController.h"

#include "core/application/dtos/PackageDTO.h"
#include "core/application/RequestContext.h"
#include "core/application/services/DeploymentService.h"
#include "drogon/HttpTypes.h"

namespace bxt::Presentation {
using namespace drogon;

drogon::Task<drogon::HttpResponsePtr>
    DeploymentController::deploy_start(drogon::HttpRequestPtr req) {
    auto const result = HttpResponse::newHttpResponse();
    result->setContentTypeCode(drogon::CT_TEXT_PLAIN);

    auto const key_it = req->headers().find("key");

    if (key_it == req->headers().cend()) {
        result->setBody("No API key");
        result->setStatusCode(drogon::k401Unauthorized);
        co_return result;
    }
    auto const key = key_it->second;

    if (key != m_options.key) {
        result->setBody("Invalid API key");
        result->setStatusCode(drogon::k401Unauthorized);
        co_return result;
    }

    auto const job_url_it = req->headers().find("job-url");

    if (job_url_it == req->headers().cend()) {
        result->setBody("No job URL");
        result->setStatusCode(drogon::k400BadRequest);
        co_return result;
    }
    auto const job_url = job_url_it->second;

    auto const start_ok = co_await m_service.deploy_start(RequestContext {
        .user_name = job_url,
    });

    if (!start_ok.has_value()) {
        result->setBody(start_ok.error().message);
        result->setStatusCode(drogon::HttpStatusCode::k400BadRequest);

        co_return result;
    }

    result->setBody(std::to_string(*start_ok));
    co_return result;
}

drogon::Task<drogon::HttpResponsePtr>
    DeploymentController::deploy_push(drogon::HttpRequestPtr req) {
    auto result = HttpResponse::newHttpResponse();
    result->setContentTypeCode(drogon::CT_TEXT_PLAIN);

    MultiPartParser file_upload;

    int ok = file_upload.parse(req);
    if (ok < 0) {
        result->setBody("Invalid request");
        result->setStatusCode(drogon::k400BadRequest);
        co_return result;
    }

    auto const headers = req->getHeaders();

    auto const session_id = std::stoull(headers.at("session"));
    auto const key = headers.find("key");

    if (key == headers.end() || key->second != m_options.key) {
        result->setBody("Invalid API key");
        result->setStatusCode(drogon::k401Unauthorized);
        co_return result;
    }

    auto const verified = co_await m_service.verify_session(session_id);

    if (!verified.has_value()) {
        result->setBody("Session is invalid");
        result->setStatusCode(drogon::k400BadRequest);
        co_return result;
    }

    auto const files_map = file_upload.getFilesMap();

    auto const file = files_map.find("file");
    auto const signature = files_map.find("signature");

    if (signature == files_map.end() || file == files_map.end()) {
        result->setBody("No package file or signature");
        result->setStatusCode(drogon::k400BadRequest);
        co_return result;
    }

    auto const params_map = file_upload.getParameters();

    auto const branch = params_map.find("branch");
    auto const repo = params_map.find("repository");
    auto const arch = params_map.find("architecture");

    if (branch == params_map.end() || repo == params_map.end() || arch == params_map.end()) {
        result->setBody("Ivalid section request");
        result->setStatusCode(drogon::k400BadRequest);
        co_return result;
    }

    auto const section = PackageSectionDTO {
        .branch = branch->second, .repository = repo->second, .architecture = arch->second};

    file->second.save();
    signature->second.save();

    auto const name = file->second.getFileName();

    auto dto = PackageDTO {section,
                           "",
                           false,
                           {{Core::Domain::PoolLocation::Automated,
                             {
                                 "",
                                 app().getUploadPath() + "/" + name,
                                 app().getUploadPath() + "/" + signature->second.getFileName(),
                             }}}

    };

    auto const push_ok = co_await m_service.deploy_push(dto, session_id);

    if (!push_ok.has_value()) {
        result->setBody(push_ok.error().error_type
                                == DeploymentService::Error::ErrorType::InvalidArgument
                            ? "Invalid section"
                            : push_ok.error().message);

        result->setStatusCode(drogon::k400BadRequest);
        co_return result;
    }

    result->setStatusCode(drogon::k200OK);
    result->setBody("ok");

    co_return result;
}

drogon::Task<drogon::HttpResponsePtr> DeploymentController::deploy_end(drogon::HttpRequestPtr req) {
    auto result = HttpResponse::newHttpResponse();
    result->setContentTypeCode(drogon::CT_TEXT_PLAIN);

    auto const headers = req->getHeaders();

    auto const session_id = std::stoull(headers.at("session"));
    auto const key = headers.find("key");

    if (key == headers.end() || key->second != m_options.key) {
        result->setBody("Invalid API key");
        result->setStatusCode(drogon::k401Unauthorized);
        co_return result;
    }

    auto const verified = co_await m_service.verify_session(session_id);

    if (!verified.has_value()) {
        result->setBody("Session is invalid");
        result->setStatusCode(drogon::k400BadRequest);
        co_return result;
    }

    auto const end_ok = co_await m_service.deploy_end(session_id);
    if (!end_ok.has_value()) {
        result->setBody(end_ok.error().message);
        result->setStatusCode(drogon::k400BadRequest);
        co_return result;
    }

    result->setStatusCode(drogon::k200OK);
    result->setBody("ok");

    co_return result;
}

} // namespace bxt::Presentation
