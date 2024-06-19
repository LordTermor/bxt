/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
import { WretchOptions } from "wretch";
import { api } from "./wretch";

export const createUsersApi = (options: WretchOptions = {}) => {
    const usersApi = api.options(options).url("/users");

    return {
        addUser: async (userData: User) => {
            const response = await usersApi.post(userData, "/add").json();
            return response;
        },

        updateUser: async (userData: Partial<User>) => {
            const response = await usersApi.patch(userData, "/update").json();
            return response;
        },

        removeUser: async (username: string) => {
            const response = await usersApi
                .delete(`/remove/${username}`)
                .json();
            return response;
        },

        getUsers: async (): Promise<User[]> => {
            const response = await usersApi.get().json<User[]>();
            return response;
        }
    };
};

export const { addUser, updateUser, removeUser, getUsers } = createUsersApi();
