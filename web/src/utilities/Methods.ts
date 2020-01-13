import {Dispatch} from "react";
import {SiteAction} from "../data/SiteContext";
import {ImageProperties} from "../components/ImageUpload";
import {MediaPaths} from "./Constants";

const timeouts:any[] = []
export function safelyTimeout(handler: ()=>void, timeout?: number): number {
    const handle = window.setTimeout(()=>{
        const index = timeouts.indexOf(handle)
        if (index) {
            timeouts.splice(index, 1);
        }
        handler()
    }, timeout)
    timeouts.push(handle)
    return handle
}

export function safelyInterval(handler: ()=>void, timeout?: number): number {
    const handle = window.setInterval(()=>{
        handler()
    }, timeout)
    timeouts.push(handle)
    return handle
}

export function clearTimeouts() {
    while(timeouts.length > 0) {
        const handle = timeouts.pop()
        clearInterval(handle)
    }
}

export function dispatchDataProperties(basePath:string, data:any, dispatch: Dispatch<SiteAction>) {
    dispatch({
        type: "setSceneData",
        sceneData: {
            backgroundUrl: basePath + "/" + data.images["main"],
            lightingUrl: basePath + "/" + data.images["lighting"],
            masks:{
                "floor": basePath + "/" + data.images["masks"]["floor"]
            },
            ancorPoint:[0,1]
        },
    })

    dispatch({
        type: "setFov",
        fov: data.fov
    })

    dispatch({
        type: "setPosition",
        position: data.cameraPosition
    })

    dispatch({
        type: "setRotation",
        rotation: [data.cameraRotation[0], data.floorRotation, data.cameraRotation[2]]
    })
}

export function dispatchImageProperties(imageProperties: ImageProperties, dispatch: Dispatch<SiteAction>) {

    //set scene data
    dispatch({
        type: "setSceneData",
        sceneData: imageProperties.scene || null
    })

    dispatch({
        type: "setFov",
        fov: imageProperties.fov || null
    })

    dispatch({
        type: "setPosition",
        position: imageProperties.cameraPosition || null
    })

    dispatch({
        type: "setRotation",
        rotation: imageProperties.cameraRotation ? [imageProperties.cameraRotation![0], imageProperties.floorRotation || 0, imageProperties.cameraRotation![2]] : null
    })
}


export function selectScene(path: string, dispatch: Dispatch<SiteAction>) {
    const jsonPath = MediaPaths.Scenes + "/" + path + "/data.json"
    fetch(jsonPath).then(res => res.json())
        .then(data => {
            dispatchDataProperties(MediaPaths.Scenes + "/" + path, data, dispatch)
        })
}

export function objectToLowerCase(object: any) {
    const newObject: any = {}

    for (const key of Object.keys(object)) {
        newObject[key.toLocaleLowerCase()] = object[key]
    }

    return newObject
}

export function cleanInput(input: string) {
    return input.replace(/\W/g, '')
}