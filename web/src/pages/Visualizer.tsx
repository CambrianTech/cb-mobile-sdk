import * as THREE from 'three'
import React, {useCallback, useContext, useEffect, useMemo, useRef} from 'react'
import 'react-dat-gui/build/react-dat-gui.css'
import './Visualizer.css'

import {
    CBMaterialProperties,
    CBVisualizer
} from "react-home-harmony";
import { SiteContext } from '../data/SiteContext';
import {ImageProperties, ImageUpload, openImageDialog} from "../components/ImageUpload";
import {VisualizerTools} from "../components/VisualizerTools";
import {dispatchImageProperties, selectScene} from "../utilities/Methods";
import {DEFAULT_SCENE} from "../utilities/Constants";

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

    const initialize = useCallback(() => {
        selectScene(DEFAULT_SCENE, dispatch)
        //Window.prototype.selectMaterial = test
    }, [dispatch])

    const initializeRef = useRef(initialize);
    useEffect(() => { initializeRef.current = initialize; }, [initialize]);

    useEffect(() => {
        if (initializeRef.current) {
            initializeRef.current()
        }
    }, []);

    const onChangeImage = useCallback(() => {
        openImageDialog()
    }, [])

    const onImageChosen = useCallback((imageProperties: ImageProperties) => {
        dispatchImageProperties(imageProperties, dispatch)
    }, [dispatch])

    const defaultMaterial = useMemo<CBMaterialProperties>(() => {
        return {
            ppi:20,
            diffuseUrl:"assets/textures/floor/narrow-floorboards1-albedo.png",
            normalsUrl:"assets/textures/floor/narrow-floorboards1-normal-dx.png",
            specularUrl:"assets/textures/floor/narrow-floorboards1-roughness.png"
        }
    }, [])

    return useMemo(() => (
        <div className={"visualizer"}>
            <CBVisualizer
                material={state.materialProperties}
                defaultMaterial={defaultMaterial}
                scene={state.sceneData}
                fov={fov}
                cameraPosition={position}
                cameraRotation={[rotation[0], 0, rotation[2]]}
                floorRotation={rotation[1]}
            />
            <VisualizerTools onChangeImage={onChangeImage} />
            <ImageUpload onImageChosen={onImageChosen}/>
        </div>
    ), [
        fov, position, rotation, state.materialProperties, state.sceneData,
        onImageChosen, onChangeImage, defaultMaterial
    ])
}
