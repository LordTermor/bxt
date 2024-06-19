/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */

import { api } from "./wretch";

const packagesApi = api.url("/packages");

export const commitTransaction = async () => {
    const response = await packagesApi.post({}, "/commit").json();
    return response;
};

export const getPackages = async (
    branch: string,
    repository: string,
    architecture: string
): Promise<Package[]> => {
    const response = await packagesApi
        .get(
            `?branch=${branch}&repository=${repository}&architecture=${architecture}`
        )
        .json<Package[]>();
    return response;
};

export const syncPackages = async () => {
    const response = await packagesApi.post({}, "/sync").json();
    return response;
};

export const snapPackages = async () => {
    const response = await packagesApi.post({}, "/snap").json();
    return response;
};
