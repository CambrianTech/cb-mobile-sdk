import * as THREE from 'three'
import React, {useCallback, useContext, useEffect, useMemo, useRef, useState} from 'react'
import 'react-dat-gui/build/react-dat-gui.css'
import './Visualizer.css'

import {
    CBCommand,
    CBMaterialProperties,
    CBSceneData,
    CBTextureLoadError,
    CBTextureLoadErrorReason,
    CBToolMode,
    CBVisualizer,
    captureScreenshot, useImageUploader,
} from "react-home-harmony";
import {BrowserType} from "react-client-info";
import { SiteContext } from '../data/SiteContext';

// Replace 3js's flooring function with ceil, so it upscales to
// powers of two instead of downscaling for sharper textures.
// Might mess with other stuff but haven't noticed anything yet.
THREE.Math.floorPowerOfTwo = THREE.Math.ceilPowerOfTwo;

enum ActivePanel {
    None = "none",
    Swatches = "swatches",
    Filters = "filters",
    Materials = "materials",
    ProductInfo = "info",
    Share = "share",
}

const shareImageSize = [1000,600]

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

    const initialize = useCallback(() => {
        
    }, [])

    const initializeRef = useRef(initialize);
    useEffect(() => { initializeRef.current = initialize; }, [initialize]);

    useEffect(() => {
        if (initializeRef.current) {
            initializeRef.current()
        }
    }, []);

    return useMemo(() => (
        <div id={"visualizer"}>
            <CBVisualizer
                material={state.materialProperties}
                defaultMaterial = {new CBMaterialProperties(20,"blue-tile.jpeg")}
                scene={state.sceneData}
                fov={fov}
                cameraPosition={position}
                cameraRotation={[rotation[0], 0, rotation[2]]}
                floorRotation={0}
            />
        </div>
    ), [
        props.history
    ])
}
