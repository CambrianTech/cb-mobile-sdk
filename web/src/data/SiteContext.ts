import { createContext, Dispatch } from "react"
import {CBMaterialProperties, CBSceneProperties} from "react-home-harmony";
import { polyfill } from "smoothscroll-polyfill"
import {BrowserProperties} from "react-client-info";
polyfill()

export type DerivedSiteState = {
    browserProperties: BrowserProperties,
    error: Error | null
}

export type SharableVisualizerState = {
    fov: number | null
    position: [number, number, number] | null
    rotation: [number, number, number] | null
    floorRotationOffset: number | null
}

export type DerivedVisualizerState = {
    sceneData:CBSceneProperties | null
    materialProperties: CBMaterialProperties | null
    onMaterialTextureChanged: ((path: string) => (void)) | null
    previewWidth: number | null
    previewHeight: number | null
    showControls?: boolean | undefined
}

export type SiteState = DerivedSiteState & SharableVisualizerState & DerivedVisualizerState

export type SiteStateContext = {
    state: SiteState
    dispatch: Dispatch<SiteAction>
}

export function createEmptyState(): SiteState {
    return {
        // shared
        browserProperties: {},
        error: null,

        // Visualizer shared
        fov: null,
        position: null,
        rotation: null,
        floorRotationOffset: null,

        // Visualizer derived
        sceneData: null,
        materialProperties: null,
        onMaterialTextureChanged: null,
        previewWidth: null,
        previewHeight: null,
        showControls: undefined
    }
}

function createUndefinedStateContext(): SiteStateContext | undefined {
    return undefined
}


export type SiteActionSetBrowserProperties = {
    type: "setBrowserProperties"
    browserProperties: BrowserProperties
}

export type SiteActionSetError = {
    type: "setError"
    error: Error | null
}

export type SiteActionSetSceneData = {
    type: "setSceneData"
    sceneData: CBSceneProperties | null
}

export type SiteActionSetFov = {
    type: "setFov"
    fov: number | null
}

export type ShawActionSetPosition = {
    type: "setPosition"
    position: [number, number, number] | null
}

export type ShawActionSetRotation = {
    type: "setRotation"
    rotation: [number, number, number] | null
}

export type SiteAction = SiteActionSetBrowserProperties | SiteActionSetError | SiteActionSetSceneData | SiteActionSetFov | ShawActionSetPosition | ShawActionSetRotation

export function siteStateReducer(state: SiteState, action: SiteAction): SiteState {
    // Set the thing we are supposed to set. Also make sure anything depending
    // on the thing we set gets reset to null.
    const newState: SiteState = { ...state }

    switch (action.type) {
        // Site underived, null everything below in the hierarchy
        case "setBrowserProperties":
            newState.browserProperties = action.browserProperties
            break
        case "setError":
            newState.error = action.error
            break
        case "setFov":
            newState.fov = action.fov
            break
        case "setPosition":
            newState.position = action.position
            break
        case "setRotation":
            newState.rotation = action.rotation
            break
        // Visualizer derived
        case "setSceneData":
            newState.sceneData = action.sceneData
            break

        default:
            throw new Error("Invalid action: " + JSON.stringify(action))
    }

    return newState
}

export const SiteContext = createContext(createUndefinedStateContext())

export function redirectKeepSearch(props: any, target: string) {
    // Important: in the useEffect in index.tsx we listen to state changes and change
    // the window's history with pushState. This is not the same history as react's
    // so we need to use the search of the window here.
    // Ideally we modify react's history in index.tsx instead.
    const url = `${target}${window.location.search}`
    props.history.push(url)
}
