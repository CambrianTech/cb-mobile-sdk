import * as THREE from 'three'
import React, {useCallback, useContext, useEffect, useMemo, useRef, Dispatch} from 'react'
import 'react-dat-gui/build/react-dat-gui.css'
import './Visualizer.css'

import {
    CBMaterialProperties,
    CBVisualizer
} from "react-home-harmony";
import { SiteContext, SiteAction, MediaPaths } from '../data/SiteContext';

export function dispatchDataProperties(basePath:string, data:any, dispatch: Dispatch<SiteAction>) {
    dispatch({
        type: "setSceneData",
        sceneData: {
            backgroundUrl: basePath + "/" + data.images["main"],
            lightingUrl: basePath + "/" + data.images["lighting"],
            masks:{
                "floor": basePath + "/" + data.images["masks"]["floor"]
            }
        }
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

export function selectScene(path: string, dispatch: Dispatch<SiteAction>) {
    fetch(MediaPaths.Scenes + path + "/data.json").then(res => res.json())
        .then(data => {
            dispatchDataProperties(MediaPaths.Scenes + path, data, dispatch)
        })
}

window.addEventListener("message", message => {
    alert("received: " + message.data)
});

// Replace 3js's flooring function with ceil, so it upscales to
// powers of two instead of downscaling for sharper textures.
// Might mess with other stuff but haven't noticed anything yet.
THREE.Math.floorPowerOfTwo = THREE.Math.ceilPowerOfTwo;

export default function Visualizer(props: any) {
    const siteContext = useContext(SiteContext)!;
    const dispatch = siteContext.dispatch
    const state = siteContext.state;

    const position = state.position || [0, 1, 0];
    const rotation = state.rotation || [0, 0, 0];
    const fov = state.fov || 60;

    let _isMounted = useRef(false);

    useEffect(() => {
        _isMounted.current = true;

        return () => {
            _isMounted.current = false
        }
    }, []);

    const test = useCallback((url:string) => {
        alert("GOT ")
    }, [])

    const initialize = useCallback(() => {
        selectScene("/dining-room/BlueRidgePine-0868V-00623-EarthPine-9in", dispatch)

        //Window.prototype.selectMaterial = test
    }, [dispatch])

    const initializeRef = useRef(initialize);
    useEffect(() => { initializeRef.current = initialize; }, [initialize]);

    useEffect(() => {
        if (initializeRef.current) {
            initializeRef.current()
        }
    }, []);

    return useMemo(() => (
        <div className={"visualizer"}>
            <CBVisualizer
                material={state.materialProperties}
                defaultMaterial = {new CBMaterialProperties(20,"blue-tile.jpeg")}
                scene={state.sceneData}
                fov={fov}
                cameraPosition={position}
                cameraRotation={[rotation[0], 0, rotation[2]]}
                floorRotation={rotation[1]}
            />
        </div>
    ), [
        fov, position, rotation, state.materialProperties, state.sceneData
    ])
}
