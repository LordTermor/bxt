/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
import { ChonkyIconName, FileArray, FileData } from "chonky";
import { useCallback, useEffect, useState } from "react";
import { SectionUtils } from "../utils/SectionUtils";
import axios from "axios";

export interface IUpdateFiles {
    (sections: Section[], path: string[]): void;
}

export const useFilesFromSections = (
    sections: Section[],
    path: string[]
): [FileArray, IUpdateFiles, Package[] | undefined] => {
    const [files, setFiles] = useState<FileArray>([]);
    const [packages, setPackages] = useState<Package[]>();

    const getPackages = async (sections: Section[], path: string[]) => {
        const value = await axios.get(`/api/packages`, {
            params: {
                branch: path[1],
                repository: path[2],
                architecture: path[3]
            }
        });
        if (value.data == null) {
            setFiles([]);
            setPackages(undefined);
            return;
        }
        setPackages(value.data);

        setFiles(
            value.data.map((pkg: any): FileData => {
                console.log(pkg);
                return {
                    id: `root/${path[1]}/${path[2]}/${path[3]}/${pkg?.name}`,
                    name: pkg.name,
                    ext: "",
                    isDir: false,
                    thumbnailUrl: pkg?.preferredLocation
                        ? pkg?.poolEntries[pkg?.preferredLocation].hasSignature
                            ? `/signature.svg`
                            : `/package.svg`
                        : "",
                    icon: ChonkyIconName.archive,
                    color: "#8B756B"
                };
            })
        );
    };

    const updateFiles = useCallback(
        (sections: Section[], path: string[]) => {
            switch (path.length) {
                case 1:
                    setFiles(
                        SectionUtils.branches(sections).map(
                            (value): FileData => {
                                return {
                                    id: `root/${value}`,
                                    name: value,
                                    isDir: true,
                                    thumbnailUrl: `/branch.svg`,
                                    color: "#8B756B"
                                };
                            }
                        )
                    );
                    break;
                case 2:
                    setFiles(
                        SectionUtils.reposForBranch(sections, path[1]).map(
                            (value): FileData => {
                                return {
                                    id: `root/${path[1]}/${value}`,
                                    name: value,
                                    isDir: true,
                                    thumbnailUrl: `/repository.svg`,
                                    color: "#8B756B"
                                };
                            }
                        )
                    );
                    break;
                case 3:
                    setFiles(
                        SectionUtils.architecturesForBranchAndRepo(
                            sections,
                            path[1],
                            path[2]
                        ).map((value): FileData => {
                            return {
                                id: `root/${path[1]}/${path[2]}/${value}`,
                                name: value,
                                isDir: true,
                                thumbnailUrl: `/architecture.svg`,
                                color: "#8B756B"
                            };
                        })
                    );
                    break;
                case 4:
                    getPackages(sections, path);
                    break;
            }
        },
        [sections, path, setFiles, setPackages]
    );

    useEffect(() => {
        updateFiles(sections, path);
    }, [sections, path]);

    return [files, updateFiles, packages];
};
