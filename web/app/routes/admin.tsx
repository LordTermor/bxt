/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2024 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
import { faPlus, faTrash } from "@fortawesome/free-solid-svg-icons";
import { FontAwesomeIcon } from "@fortawesome/react-fontawesome";
import { useCallback, useRef, useState } from "react";
import { Table, Button } from "react-daisyui";
import UserModal, { UserModalProps } from "../components/UserModal";
import { useQueryClient, useQuery, useMutation } from "@tanstack/react-query";
import {
    addUser,
    createUsersApi,
    getUsers,
    removeUser,
    updateUser
} from "~/api/users";
import { json, useLoaderData } from "@remix-run/react";
import { LoaderFunctionArgs } from "@remix-run/node";

export const loader = async ({ request }: LoaderFunctionArgs) => {
    console.log(request.headers.get("Cookie"));
    const api = createUsersApi({
        headers: { cookies: request.headers.get("Cookie") }
    });
    const users = await api.getUsers();
    return json({ users });
};

export default () => {
    const { users } = useLoaderData<typeof loader>();
    const queryClient = useQueryClient();

    // Fetch users
    const { data: usersData, isLoading } = useQuery<User[]>({
        queryKey: ["users"],
        queryFn: getUsers,
        initialData: users
    });

    // Add user mutation
    const addUserMutation = useMutation({
        mutationFn: addUser,
        onSuccess: () =>
            queryClient.invalidateQueries({
                queryKey: ["users"],
                type: "active"
            })
    });

    // Update user mutation
    const updateUserMutation = useMutation({
        mutationFn: updateUser,
        onSuccess: () =>
            queryClient.invalidateQueries({
                queryKey: ["users"],
                type: "active"
            })
    });

    // Remove user mutation
    const removeUserMutation = useMutation({
        mutationFn: removeUser,
        onSuccess: () =>
            queryClient.invalidateQueries({
                queryKey: ["users"],
                type: "active"
            })
    });

    const userModalRef = useRef<HTMLDialogElement>(null);
    const [userModalProps, setUserModalProps] = useState<UserModalProps>({
        backdrop: true
    });

    const openUserModal = useCallback(
        (user?: User) => {
            setUserModalProps({ ...userModalProps, user });
            userModalRef.current?.showModal();
        },
        [userModalProps, setUserModalProps, userModalRef]
    );

    if (isLoading) {
        return <div>Loading...</div>;
    }

    return (
        <div className="px-2 w-full h-full">
            <UserModal
                onSaveClicked={(user, isNew) => {
                    if (user) {
                        if (isNew) {
                            addUserMutation.mutate(user);
                        } else {
                            updateUserMutation.mutate(user);
                        }
                    }
                    userModalRef.current?.close();
                }}
                {...userModalProps}
                ref={userModalRef}
            />
            <Table zebra={true} className="rounded-none">
                <thead>
                    <th style={{ width: "90%" }}>Name</th>
                    <th style={{ width: "10%" }}></th>
                </thead>
                <Table.Body>
                    {users?.map((value) => (
                        <Table.Row
                            style={{ userSelect: "none" }}
                            onDoubleClick={() => openUserModal(value)}
                        >
                            <span>{value.name}</span>

                            <Button
                                color="ghost"
                                onClick={() =>
                                    removeUserMutation.mutate(value.name)
                                }
                            >
                                <FontAwesomeIcon icon={faTrash} />
                            </Button>
                        </Table.Row>
                    ))}
                </Table.Body>
            </Table>
            <div className="fixed bottom-6 right-6 space-x-4">
                <Button color="accent" onClick={() => openUserModal()}>
                    <FontAwesomeIcon icon={faPlus} />
                    New user
                </Button>
            </div>
        </div>
    );
};
