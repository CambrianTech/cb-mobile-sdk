import * as THREE from 'three'
import React, {useCallback, useContext, useEffect, useMemo, useRef, useState} from 'react'
import 'react-dat-gui/build/react-dat-gui.css'
import './Visualizer.css'

import {CBVisualizer} from "react-home-harmony";
import { SiteContext } from '../data/SiteContext';
import {ImageProperties, openImageDialog} from "../components/ImageUpload";
import {dispatchImageProperties, objectToLowerCase, selectScene} from "../utilities/Methods";
import {DEFAULT_MATERIAL, DEFAULT_SCENE, GRID_MATERIAL} from "../utilities/Constants";
import * as qs from "querystring";

const VisualizerTools = React.lazy(() => import('../components/VisualizerTools'));
const ImageUpload = React.lazy(() => import('../components/ImageUpload'));

// Replace 3js's flooring function with ceil, so it upscales to
// powers of two instead of downscaling for sharper textures.
// Might mess with other stuff but haven't noticed anything yet.
THREE.Math.floorPowerOfTwo = THREE.Math.ceilPowerOfTwo;

export default function Visualizer(props: any) {
    const siteContext = useContext(SiteContext)!;
    const dispatch = siteContext.dispatch
    const state = siteContext.state;

    const [canLoad, setCanLoad] = useState(false)
    const searchObject = objectToLowerCase(qs.parse(window.location.search.substr(1)))

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
        if (!state.sceneData && !searchObject.scene && !searchObject.wait) {
            selectScene(DEFAULT_SCENE, dispatch)
        }
    }, [dispatch, searchObject.scene, searchObject.wait, state.sceneData])

    useEffect(() => {
        setCanLoad(true)
    }, [state.sceneData]);

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
        setCanLoad(true)
    }, [dispatch])

    return useMemo(() => (
        <div className={"visualizer"}>
            <CBVisualizer
                material={state.materialProperties}
                defaultMaterial={state.showControls ? GRID_MATERIAL : DEFAULT_MATERIAL}
                showControls={state.showControls}
                scene={state.sceneData}
                fov={fov}
                cameraPosition={position}
                cameraRotation={[rotation[0], 0, rotation[2]]}
                floorRotation={rotation[1]}
                canLoad={canLoad}
            />
            {!state.showControls && <VisualizerTools onChangeImage={onChangeImage} />}
            <ImageUpload onImageChosen={onImageChosen}/>
        </div>
    ), [state.materialProperties, state.showControls, state.sceneData, fov, position, rotation, canLoad, onChangeImage, onImageChosen])
}
