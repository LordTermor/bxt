/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */

import { useState, useEffect } from "react";
import { useCompareResults, useSections } from "~/hooks/BxtHooks";
import { Button, Loading } from "react-daisyui";
import CompareTable from "~/components/compare/CompareTable";
import CompareInputForm from "~/components/compare/CompareInputForm";
import { FontAwesomeIcon } from "@fortawesome/react-fontawesome";
import { faCodeCompare } from "@fortawesome/free-solid-svg-icons";

export default (props: any) => {
    const [sections, updateSections] = useSections();
    const [compareResults, getCompareResults, resetCompareResults] =
        useCompareResults();

    const [isLoading, setIsLoading] = useState<boolean>(false);

    useEffect(() => {
        setIsLoading(false);
    }, [compareResults]);

    return (
        <div className="w-full h-full bg-base-200 overflow-x-auto">
            {compareResults ? (
                isLoading ? (
                    <div className="grow flex w-full justify-center content-center">
                        <Loading variant="bars" className="w-20" />
                    </div>
                ) : (
                    <div>
                        <CompareTable compareResults={compareResults} />
                        <Button
                            color="accent"
                            className="fixed bottom-6 right-6"
                            onClick={resetCompareResults}
                        >
                            <FontAwesomeIcon icon={faCodeCompare} />
                            New compare
                        </Button>
                    </div>
                )
            ) : (
                <div className="overflow-y-auto flex h-full w-full justify-center items-center">
                    <CompareInputForm
                        sections={sections}
                        getCompareResults={(sections) => {
                            setIsLoading(true);
                            getCompareResults(sections);
                        }}
                    />
                </div>
            )}
        </div>
    );
};
