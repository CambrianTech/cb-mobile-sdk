import 'react-app-polyfill/ie9'
import 'react-app-polyfill/stable'
import cssVars from 'css-vars-ponyfill'

import React, {useReducer, useEffect, useCallback, useState} from "react"
import * as ReactDOM from "react-dom"

import { BrowserRouter as Router, Route, Switch } from "react-router-dom"
import { SiteContext, createEmptyState, siteStateReducer } from "./data/SiteContext"
import Visualizer from "./pages/Visualizer"
import {BrowserProperties, WebClientInfo} from "react-client-info"
import {cbInitialize} from "react-home-harmony";

import 'react-circular-progressbar/dist/styles.css'
import '@material/react-button/dist/button.css';
import '@material/react-fab/dist/fab.css';
import {ProductDetails} from "./pages/ProductDetails";
import {BrandInfo} from "./pages/BrandInfo";
import * as qs from "querystring";
import {objectToLowerCase} from "./utilities/Methods";

const objectFitImages = require('object-fit-images')

if (process.env.REACT_APP_CB_GET_UPLOAD_URLS_URL && process.env.REACT_APP_CB_UPLOADS_URL && process.env.REACT_APP_CB_SEGMENT_URL) {
    cbInitialize({
        uploadUrl: process.env.REACT_APP_CB_GET_UPLOAD_URLS_URL,
        projectHostingUrl: process.env.REACT_APP_CB_UPLOADS_URL,
        processingUrl: process.env.REACT_APP_CB_SEGMENT_URL
    })
} else {
    throw new Error('REACT_APP_CB_GET_UPLOAD_URLS_URL, REACT_APP_CB_UPLOADS_URL, and REACT_APP_CB_SEGMENT_URL must be defined')
}

function App() {
    const initialSiteState = createEmptyState()
    const [siteState, dispatchSiteState] = useReducer(siteStateReducer, initialSiteState)
    const [firstRender, setFirstRender] = useState(true)
    const [browserProperties, setBrowserProperties] = useState<BrowserProperties>({})
    // Url load states

    //component mounted:
    useEffect(() => {
        cssVars()
        objectFitImages()

        return () => {
            //unmount
        }
    }, [])

    useEffect(() => {
        if (browserProperties.hasTouchpad) {
            document.documentElement.style.setProperty("--scrollbar-style", "none")
            document.documentElement.style.setProperty("--scrollbar-display", "none")
            document.documentElement.style.setProperty("--scrollbar-thickness", "0px")
        }
    }, [browserProperties.hasTouchpad])

    const setCssVars = useCallback(() => {
        if (!browserProperties.browser) return;

        const doc = document.documentElement;

        //without this check, causes WebGL flicker on desktop
        const width = window.innerWidth;
        const height = window.innerHeight;

        doc.style.setProperty("--app-height",  `${height}px`);
        doc.style.setProperty("--inverse-app-height", `${-height}px`);
        doc.style.setProperty("--half-app-height", `${height / 2}px`);
        doc.style.setProperty("--inverse-half-app-height", `${-height / 2}px`);
        doc.style.setProperty("--app-width", `${width}px`);
        doc.style.setProperty("--inverse-app-width", `${-width}px`)

    }, [browserProperties.browser])

    useEffect(() => {
        dispatchSiteState({ type: "setBrowserProperties", browserProperties: browserProperties })
        setCssVars()
    }, [browserProperties, setCssVars])

    function updateFromLocation(location: any) {

        // Parse URL search string without the first character (typically question mark).
        // Also turn the keys into lowercase so their case doesn't matter.
        const searchObject = objectToLowerCase(qs.parse(location.search.substr(1)))

        const searchFov = searchObject.f as string
        if (searchFov) {
            const fov = parseFloat(searchFov)
            if (!siteState.fov || (fov - siteState.fov) > 0.0001) {
                dispatchSiteState({
                    type: "setFov",
                    fov: fov
                })
            }
        }

        const searchPosX = searchObject.px as string
        const searchPosY = searchObject.py as string
        const searchPosZ = searchObject.pz as string
        if (searchPosX && searchPosY && searchPosZ) {
            const px = parseFloat(searchPosX)
            const py = parseFloat(searchPosY)
            const pz = parseFloat(searchPosZ)

            if (!siteState.position || siteState.position[0] - px > 0.0001 || siteState.position[1] - py > 0.0001 || siteState.position[2] - pz > 0.0001) {
                dispatchSiteState({
                    type: "setPosition",
                    position: [px, py, pz]
                })
            }
        }

        const searchRotX = searchObject.rx as string
        const searchRotY = searchObject.ry as string
        const searchRotZ = searchObject.rz as string
        if (searchRotX && searchRotY && searchRotZ) {
            const rx = parseFloat(searchRotX)
            const ry = parseFloat(searchRotY)
            const rz = parseFloat(searchRotZ)

            if (!siteState.rotation || siteState.rotation[0] - rx > 0.0001 || siteState.rotation[1] - ry > 0.0001 || siteState.rotation[2] - rz > 0.0001) {
                dispatchSiteState({
                    type: "setRotation",
                    rotation: [rx, ry, rz]
                })
            }
        }

        const floorRotationOffset = searchObject.fro as string
        if (floorRotationOffset) {
            dispatchSiteState({
                type: "setFloorRotationOffset",
                floorRotationOffset: parseFloat(floorRotationOffset)
            })
        }

        if (searchObject.controls) {
            dispatchSiteState({
                type: "setShowControls",
                showControls: searchObject.controls
            })
        }
    }

    return (
        <Router>
            <Route
                render={({ location }) => {
                    if (firstRender) {
                        setFirstRender(false)
                        updateFromLocation(location)
                    }

                    return (
                        <SiteContext.Provider value={{ state: siteState, dispatch: dispatchSiteState }}>

                            <WebClientInfo onClientStateChanged={setBrowserProperties} />

                            <Switch location={location}>
                                <Route exact path="/" component={Visualizer} />
                                <Route exact path="/product-details" component={ProductDetails} />
                                <Route exact path="/brand-info" component={BrandInfo} />
                            </Switch>

                        </SiteContext.Provider>
                    )
                }}
            />
        </Router>
    )
}

ReactDOM.render(
    <App />,
    document.getElementById("root")
)