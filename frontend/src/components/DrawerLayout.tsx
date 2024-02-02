import { Drawer, Menu, Button, Progress } from "react-daisyui";
import { Link, Outlet, useLocation } from "react-router-dom";
import ConfirmSyncModal from "./ConfirmSyncModal";
import { useCallback, useMemo, useRef } from "react";
import { useLocalStorage } from "@uidotdev/usehooks";
import axios from "axios";
import { useSyncMessage } from "../hooks/BxtWebSocketHooks";
import { FontAwesomeIcon } from "@fortawesome/react-fontawesome";
import {
    faCircleDown,
    faCodeCompare,
    faFolderTree,
    faListCheck,
    faRightFromBracket,
    faToolbox
} from "@fortawesome/free-solid-svg-icons";

const triggerSync = async () => {
    await axios.post("/api/sync");
};

export default () => {
    let modalRef = useRef<HTMLDialogElement>(null);
    const [userName, setUserName] = useLocalStorage("username", null);
    const syncInProgress = useSyncMessage()?.started;

    const handleShow = useCallback(() => {
        modalRef.current?.showModal();
    }, [modalRef]);

    const routes = useMemo(
        () => [
            { route: "/", name: "Packages", icon: faFolderTree },
            { route: "/compare", name: "Compare", icon: faCodeCompare },
            { route: "/logs", name: "Logs", icon: faListCheck },
            { route: "/admin", name: "Admin", icon: faToolbox }
        ],
        []
    );

    const location = useLocation();

    return (
        <Drawer
            className="lg:drawer-open"
            open={true}
            side={
                <Menu className="h-screen p-4 w-60 bg-base-100 text-base-content border-r-base border-r-2">
                    <Link
                        to="/"
                        className="flex justify-center relative  h-14  overflow-hidden pb-2/3"
                    >
                        <img
                            id="bxt-logo"
                            src={`${process.env.PUBLIC_URL}/logo-full.png`}
                            className="absolute h-full w-40 object-contain"
                        />
                    </Link>

                    <div className="h-6"></div>

                    {routes.map(({ route, name, icon }) => (
                        <Menu.Item>
                            <Link
                                className={
                                    location.pathname == route ? "active" : ""
                                }
                                to={route}
                            >
                                <FontAwesomeIcon icon={icon} />
                                {name}
                            </Link>
                        </Menu.Item>
                    ))}

                    <div className="grow"></div>

                    {syncInProgress ? (
                        <div className="font-bold text-center">
                            Sync is in progress
                            <Progress />
                        </div>
                    ) : (
                        <Button size="sm" onClick={handleShow} color="accent">
                            <FontAwesomeIcon icon={faCircleDown} />
                            Sync
                        </Button>
                    )}

                    <div className="h-5 flex flex-col place-content-center">
                        <hr />
                    </div>
                    <Menu.Item onClick={(e) => setUserName(null)}>
                        <a>
                            <FontAwesomeIcon icon={faRightFromBracket} />
                            Logout
                        </a>
                    </Menu.Item>
                </Menu>
            }
        >
            <ConfirmSyncModal
                onCancel={() => modalRef.current?.close()}
                onConfirm={() => {
                    triggerSync();
                    modalRef.current?.close();
                }}
                ref={modalRef}
            />

            <Outlet />
        </Drawer>
    );
};
