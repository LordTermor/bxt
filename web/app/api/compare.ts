/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */

import { api } from "./wretch";

const compareApi = api.url("/compare");

export const compare = async (
    compareData: Section[]
): Promise<CompareResult> => {
    const response = await compareApi.post(compareData).json<CompareResult>();
    return response;
};
