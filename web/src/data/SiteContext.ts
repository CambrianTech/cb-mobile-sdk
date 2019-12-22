import { createContext, Dispatch } from "react"
import {CBMaterialProperties, CBSceneProperties} from "react-home-harmony";
import { polyfill } from "smoothscroll-polyfill"
polyfill()

export type DerivedSiteState = {
    isPortrait: boolean,
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

export enum MediaPaths {
    Shared = "assets/img/shared",
    Home = "assets/img/home",
    Visualizer = "assets/img/visualizer",
    Video = "assets/video",
    Employees = "assets/img/employee-profiles",
    LicenseLogos = "assets/img/logos",
}

export type SiteStateContext = {
    state: SiteState
    dispatch: Dispatch<SiteAction>
}

export function createEmptyState(): SiteState {
    return {
        // shared
        isPortrait: false,
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


export type SiteActionSetIsPortrait = {
    type: "setIsPortrait"
    isPortrait: boolean
}

export type SiteActionSetError = {
    type: "setError"
    error: Error | null
}

export type SiteAction = SiteActionSetIsPortrait | SiteActionSetError

export function siteStateReducer(state: SiteState, action: SiteAction): SiteState {
    // Set the thing we are supposed to set. Also make sure anything depending
    // on the thing we set gets reset to null.
    const newState: SiteState = { ...state }

    switch (action.type) {
        // Site underived, null everything below in the hierarchy
        case "setIsPortrait":
            newState.isPortrait = action.isPortrait
            break
        case "setError":
            newState.error = action.error
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
