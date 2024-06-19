/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */

import { api } from "./wretch";

const authApi = api.url("/auth");

export const auth = async (credentials: { name: string; password: string }) => {
    const response = await authApi
        .post({ ...credentials, response_type: "cookie" })
        .res();

    return response;
};

export const refresh = async () => {
    const response = await authApi.get("/refresh").res();
    return response;
};

export const revoke = async (token: string) => {
    const response = await authApi.post({ token }, "/revoke").res();
    return response;
};
