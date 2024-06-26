import React from "react";
import ReactDOM from "react-dom/client";
import { AppRoot } from "./root.tsx";
import "./index.css";
import "./tailwind.css";

ReactDOM.createRoot(document.getElementById("root")!).render(
    <React.StrictMode>
        <AppRoot />
    </React.StrictMode>
);
