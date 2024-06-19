/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
import React, { useState } from "react";
import { Card, Button } from "react-daisyui";
import { FontAwesomeIcon } from "@fortawesome/react-fontawesome";
import {
    faTrash,
    faPlus,
    faCodeCompare
} from "@fortawesome/free-solid-svg-icons";
import SectionSelect from "../SectionSelect";

type CompareInputFormProps = {
    sections: Section[];
    getCompareResults: (selectedSections: Section[]) => void;
};

const CompareInputForm = ({
    sections,
    getCompareResults
}: CompareInputFormProps) => {
    const [selectedSections, setSelectedSections] = useState<Section[]>([]);

    const updateSectionAt = (index: number, update: any) => {
        setSelectedSections((prevSections) => [
            ...prevSections.slice(0, index),
            { ...sections[0], ...prevSections[index], ...update },
            ...prevSections.slice(index + 1)
        ]);
    };

    return (
        <Card className="m-auto bg-base-100 shadow-sm" compact={true}>
            <Card.Body>
                <Card.Title>Compare sections</Card.Title>
                Choose sections you wish to compare.
                <span className="pt-2 px-10 space-y-4 flex-col justify-center">
                    {selectedSections.map((section, index) => (
                        <div
                            className="flex items-end h-fit -mr-14"
                            key={index}
                        >
                            <SectionSelect
                                sections={sections}
                                selectedSection={section}
                                onSelected={(section) =>
                                    updateSectionAt(index, section)
                                }
                            />

                            <Button
                                className={`mx-2 ${
                                    index + 1 == selectedSections.length &&
                                    selectedSections.length > 1
                                        ? ""
                                        : "invisible"
                                }`}
                                size="sm"
                                color="ghost"
                                onClick={() =>
                                    setSelectedSections([
                                        ...selectedSections.slice(0, index),
                                        ...selectedSections.slice(index + 1)
                                    ])
                                }
                            >
                                <FontAwesomeIcon icon={faTrash} />
                            </Button>
                        </div>
                    ))}
                    <Button
                        className="w-full"
                        color="ghost"
                        size="sm"
                        onClick={() =>
                            setSelectedSections([...selectedSections, {}])
                        }
                    >
                        <FontAwesomeIcon className="mr-2" icon={faPlus} />
                        Add section
                    </Button>
                </span>
                <Card.Actions className="justify-end">
                    <Button
                        size="sm"
                        color="accent"
                        onClick={() => {
                            getCompareResults(selectedSections);
                        }}
                    >
                        <FontAwesomeIcon
                            className="mr-2"
                            icon={faCodeCompare}
                        />
                        Compare
                    </Button>
                </Card.Actions>
            </Card.Body>
        </Card>
    );
};

export default CompareInputForm;
