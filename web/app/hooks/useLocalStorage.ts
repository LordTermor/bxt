/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */

import { useEffect, useState } from "react";

export function useLocalStorage<T>(
    key: string
): [T | null | undefined, (state: T | null) => void] {
    const [state, setState] = useState<T | null | undefined>(undefined);

    useEffect(() => {
        const value = localStorage.getItem(key);
        const state: T = value != null ? JSON.parse(value) : null;

        setState(state);
    }, [key, setState]);

    const setWithLocalStorage = (nextState: T | null) => {
        localStorage.setItem(key, JSON.stringify(nextState));
        setState(nextState);
    };

    return [state, setWithLocalStorage];
}
