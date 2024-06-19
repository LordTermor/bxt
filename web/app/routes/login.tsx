/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */

import { ActionFunctionArgs, LoaderFunctionArgs } from "@remix-run/node";
import { redirect, useSubmit } from "@remix-run/react";
import { userInfo } from "os";

import { useCallback, useState } from "react";
import { Hero, Button, Card, Form, Input } from "react-daisyui";
import { auth } from "~/api/auth";
import { createUsersApi } from "~/api/users";
import cookie from "cookie";

type LoginFormProps = {
    setUserName: (userName: string) => void;
};

export default (props: any) => {
    const [password, setPassword] = useState("");
    const [name, setName] = useState("");

    const [showPassword, setShowPassword] = useState<boolean>(false);
    const switchShowPassword = useCallback(() => {
        setShowPassword(!showPassword);
    }, [showPassword, setShowPassword]);
    const submit = useSubmit();
    return (
        <div
            style={{
                backgroundImage: `url(background.png)`
            }}
            className="bg-cover flex w-full component-preview p-4 items-center justify-center gap-2 font-sans"
        >
            <Hero className="grid h-screen place-items-center" {...props}>
                <Hero.Content className="flex-col lg:flex-row-reverse">
                    <Card className="flex-shrink-0 w-full max-w-sm shadow-2xl bg-base-100">
                        <Card.Body>
                            <div className="justify-start relative h-12 overflow-hidden pb-2/3">
                                <img
                                    src={`logo-full.png`}
                                    className="absolute h-full w-full object-contain"
                                />
                            </div>
                            <Form
                                onSubmit={(e) => {
                                    e.preventDefault();
                                    submit(
                                        { name: name, password: password },
                                        {
                                            method: "post",
                                            action: "/login"
                                        }
                                    );
                                }}
                            >
                                <Form.Label title="Login" />
                                <Input
                                    value={name}
                                    onChange={(e) => setName(e.target.value)}
                                    type="text"
                                    placeholder="login"
                                    className="input-bordered"
                                />
                                <Form.Label title="Password" />
                                <Input
                                    value={password}
                                    onChange={(e) =>
                                        setPassword(e.target.value)
                                    }
                                    type={showPassword ? "text" : "password"}
                                    placeholder="password"
                                    className="input-bordered"
                                ></Input>
                                <Form.Label />
                                <Button
                                    type="submit"
                                    value="submit"
                                    color="primary"
                                >
                                    Login
                                </Button>
                            </Form>
                        </Card.Body>
                    </Card>
                </Hero.Content>
            </Hero>
        </div>
    );
};

function parseCookies(cookieString: string): { [key: string]: string } {
    const cookies: { [key: string]: string } = {};
    if (cookieString) {
        cookieString.split(";").forEach((cookie) => {
            const parts = cookie.split("=");
            const key = parts[0].trim();
            const value = parts[1] ? parts[1].trim() : "";
            cookies[key] = decodeURIComponent(value);
        });
    }
    return cookies;
}

export async function action({ request }: ActionFunctionArgs) {
    const formData = await request.formData();

    const credentials = {
        name: formData.get("name")?.toString() || "",
        password: formData.get("password")?.toString() || ""
    };

    if (credentials.name === "" || credentials.password === "") {
        return null;
    }

    const authRes = await auth(credentials);

    if (authRes.status !== 200) {
        return null;
    }
    const cookies = cookie.parse(authRes.headers.get("set-cookie") || "");

    const access_token = cookie.parse(authRes.headers.get("set-cookie") || "")[
        "access_token"
    ];

    return redirect("/", {
        headers: {
            "Set-Cookie": cookie.serialize("access_token", access_token, {
                path: "/"
            })
        }
    });
}

export async function loader({ request }: LoaderFunctionArgs) {
    const access_token = cookie.parse(request.headers.get("cookie") || "")[
        "access_token"
    ];

    if (access_token) {
        return redirect("/");
    }

    return null;
}
