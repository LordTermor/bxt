/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */

import {
    createColumnHelper,
    flexRender,
    getCoreRowModel,
    getPaginationRowModel,
    getSortedRowModel,
    useReactTable
} from "@tanstack/react-table";
import { useNavigate } from "@remix-run/react";
import { useMemo } from "react";
import SectionLabel from "../SectionLabel";
import { Table } from "react-daisyui";

export default function CompareTable({
    compareResults
}: {
    compareResults: CompareResult;
}) {
    const navigate = useNavigate();

    const data = useMemo(
        () => compareResults?.compareTable || [],
        [compareResults]
    );

    const columnHelper = createColumnHelper<CompareEntry>();

    const columns = useMemo(
        () => [
            columnHelper.accessor("name", {
                id: "0",
                header: "Name"
            }),
            ...(compareResults?.sections.map((section, index) =>
                columnHelper.accessor(
                    (compare: CompareEntry) => {
                        if (
                            section.branch &&
                            section.repository &&
                            section.architecture &&
                            compare[
                                `${section.branch}/${section.repository}/${section.architecture}`
                            ]
                        )
                            return Object.values(
                                compare[
                                    `${section.branch}/${section.repository}/${section.architecture}`
                                ]
                            );
                    },
                    {
                        id: `${index + 1}`,
                        header: (props) => (
                            <SectionLabel {...props} section={section} />
                        )
                    }
                )
            ) || [])
        ],
        [compareResults]
    );
    const table = useReactTable<CompareEntry>({
        columns,
        data,
        manualPagination: true,
        getCoreRowModel: getCoreRowModel(),
        getPaginationRowModel: getPaginationRowModel(),
        getSortedRowModel: getSortedRowModel()
    });

    return (
        <div className="relative">
            <Table
                zebra={true}
                size="xs"
                className="w-full bg-base-100 rounded-none"
            >
                <Table.Head>
                    {table
                        .getHeaderGroups()
                        .flatMap((headerGroup) =>
                            headerGroup.headers.map((header) => (
                                <span key={header.id}>
                                    {header.isPlaceholder
                                        ? null
                                        : flexRender(
                                              header.column.columnDef.header,
                                              header.getContext()
                                          )}
                                </span>
                            ))
                        )}
                </Table.Head>
                <Table.Body>
                    {table.getRowModel().rows.map((row) => (
                        <Table.Row key={row.id}>
                            {row.getVisibleCells().map((cell) => (
                                <span key={cell.id}>
                                    {flexRender(
                                        cell.column.columnDef.cell,
                                        cell.getContext()
                                    )}
                                </span>
                            ))}
                        </Table.Row>
                    ))}
                </Table.Body>
            </Table>
        </div>
    );
}
