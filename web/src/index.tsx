import 'react-app-polyfill/ie9'
import 'react-app-polyfill/stable'
import cssVars from 'css-vars-ponyfill'

import React, {useReducer, useEffect, useCallback, useState} from "react"
import * as ReactDOM from "react-dom"

import { BrowserRouter as Router, Route, Switch } from "react-router-dom"
import { SiteContext, createEmptyState, siteStateReducer } from "./data/SiteContext"
import Visualizer from "./pages/Visualizer"
import {BrowserProperties, WebClientInfo} from "react-client-info"

const objectFitImages = require('object-fit-images')

function App() {
    const initialSiteState = createEmptyState()
    const [siteState, dispatchSiteState] = useReducer(siteStateReducer, initialSiteState)
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

    return (
        <Router>
            <Route
                render={({ location }) => {

                    return (
                        <SiteContext.Provider value={{ state: siteState, dispatch: dispatchSiteState }}>

                            <WebClientInfo onClientStateChanged={setBrowserProperties} />

                            <Switch location={location}>
                                <Route exact path="/" component={Visualizer} />
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