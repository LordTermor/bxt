/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
import wretch from "wretch";
import { retry } from "wretch/middlewares/retry";

export const api = wretch("http://localhost:8080")
    .url("/api")
    .middlewares([
        retry({
            maxAttempts: 1
        })
    ]);
