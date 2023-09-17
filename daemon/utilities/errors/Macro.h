/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#define BXT_DECLARE_RESULT(error)        \
    template<typename TBxtExpectedValue> \
    using Result = nonstd::expected<TBxtExpectedValue, error>;