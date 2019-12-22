import 'react-app-polyfill/ie9'
import 'react-app-polyfill/stable'
import cssVars from 'css-vars-ponyfill'

import React, { useReducer, useEffect, useState } from "react"
import * as ReactDOM from "react-dom"

import { BrowserRouter as Router, Route, Switch } from "react-router-dom"
import { SiteContext, createEmptyState, siteStateReducer } from "./data/SiteContext"
import Visualizer from "./pages/Visualizer"
import { BrowserProperties, WebClientMonitor } from "react-client-info"

const objectFitImages = require('object-fit-images')

function App() {
    const initialSiteState = createEmptyState()
    const [siteState, dispatchSiteState] = useReducer(siteStateReducer, initialSiteState)

    // Url load states
    const [browserProperties, setBrowserProperties] = useState<BrowserProperties>({})

    //component mounted:
    useEffect(() => {
        cssVars()
        objectFitImages()

        return () => {
            //unmount
        }
    }, [])

    return (
        <Router>
            <Route
                render={({ location }) => {

                    return (
                        <SiteContext.Provider value={{ state: siteState, dispatch: dispatchSiteState }}>

                            <WebClientMonitor onClientStateChanged={setBrowserProperties} />

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