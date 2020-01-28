import * as THREE from 'three'
import React, {useCallback, useContext, useEffect, useMemo, useRef, useState} from 'react'
import 'react-dat-gui/build/react-dat-gui.css'
import './Visualizer.css'

import {CBSceneData, CBToolMode, CBVisualizer} from "react-home-harmony";
import { SiteContext } from '../data/SiteContext';
import {ImageProperties, openImageDialog} from "../components/ImageUpload";
import {dispatchImageProperties, objectToLowerCase, selectScene} from "../utilities/Methods";
import {DEFAULT_MATERIAL, DEFAULT_SCENE, GRID_MATERIAL, MediaPaths} from "../utilities/Constants";
import * as qs from "querystring";
import {api} from "../index";
import {BrowserType} from "react-client-info";

const VisualizerTools = React.lazy(() => import('../components/VisualizerTools'));
const ImageUpload = React.lazy(() => import('../components/ImageUpload'));

// Replace 3js's flooring function with ceil, so it upscales to
// powers of two instead of downscaling for sharper textures.
// Might mess with other stuff but haven't noticed anything yet.
THREE.Math.floorPowerOfTwo = THREE.Math.ceilPowerOfTwo;

export default function Visualizer(props: any) {
    const siteContext = useContext(SiteContext)!;
    const dispatch = siteContext.dispatch
    const state = siteContext.state
    const scene = useRef<CBSceneData>()

    const [canLoad, setCanLoad] = useState(false)
    const searchObject = objectToLowerCase(qs.parse(window.location.search.substr(1)))

    const position = state.position || [0, 1, 0];
    const rotation = state.rotation || [0, 0, 0];
    const fov = state.fov || 60;

    const [isToolOverlayOpen, setIsToolOverlayOpen] = useState(false);
    const [rotationControlActive, setRotationControlActive] = useState(false);
    const [rotationControlValue, setRotationControlValue] = useState(0); // Temporary rotation offset (not applied yet)

    const [toolMode, setToolMode] = useState(CBToolMode.Select);
    const [historySize, setHistorySize] = useState<number>(0);

    let _isMounted = useRef(false);

    useEffect(() => {
        _isMounted.current = true;

        return () => {
            _isMounted.current = false
        }
    }, []);

    useEffect(() => {
        if (state.sceneData) {
            setCanLoad(true)
            if (api.onSceneLoad) {
                api.onSceneLoad()
            }
        }
    }, [state.sceneData]);

    const initialize = useCallback(() => {
        if (!state.sceneData && !searchObject.scene && !searchObject.wait) {
            selectScene(DEFAULT_SCENE, dispatch)
        }
        if (api.notifyLoaded) {
            api.notifyLoaded("Visualizer")
        }
    }, [dispatch, searchObject.scene, searchObject.wait, state.sceneData])

    const initializeRef = useRef(initialize);
    useEffect(() => { initializeRef.current = initialize; }, [initialize]);

    useEffect(() => {
        if (initializeRef.current) {
            initializeRef.current()
        }
    }, []);

    const sceneLoaded = useCallback((data: CBSceneData, methods) => {
        api.current = methods
        scene.current = data
    }, []);

    const sceneRendered = useCallback((data: CBSceneData) => {
        scene.current = data
    }, []);

    const onChangeImage = useCallback(() => {
        openImageDialog()
    }, [])

    const onImageChosen = useCallback((imageProperties: ImageProperties) => {
        dispatchImageProperties(imageProperties, dispatch)
        setCanLoad(true)
    }, [dispatch])

    const toolsShowHideButtons = useCallback((show: boolean) => {
        if (!_isMounted.current) return
        setIsToolOverlayOpen(!show)
    }, []);

    const rotateChanged = useCallback((radians: number) => {
        if (!_isMounted.current) return
        setRotationControlActive(true);
        setRotationControlValue(radians)
    }, []);

    const rotateFinished = useCallback((commit: boolean, radians: number) => {
        if (!_isMounted.current) return
        if (commit) {
            dispatch({
                type: "setFloorRotationOffset",
                floorRotationOffset: radians
            })
        }
        setRotationControlActive(false)

    }, [dispatch]);

    const isUploadedImage = useCallback(() => {
        if (state.sceneData) {
            return state.sceneData.backgroundUrl.indexOf(MediaPaths.Scenes) < 0
        }
        return false
    }, [state.sceneData]);

    const canEditArea = useCallback(() => {
        if (state.browserProperties.browser !== BrowserType.LegacyIE) {
            return isUploadedImage()
        }
        return false
    }, [isUploadedImage, state.browserProperties.browser]);

    const editAreaClicked = useCallback(() => {
        if (!_isMounted.current) return
        setToolMode(CBToolMode.Draw)
    }, [setToolMode]);

    const drawingToolChanged = useCallback((mode: CBToolMode) => {
        if (!_isMounted.current) return
        setToolMode(mode)
    }, [setToolMode]);

    const drawingToolUndo = useCallback(() => {
        if (!_isMounted.current || !api.current) return
        api.current.undoLast()
    }, []);

    const historyChanged = useCallback((length: number) => {
        if (!_isMounted.current) return
        setHistorySize(length)
    }, []);

    const drawingFinished = useCallback((commit: boolean) => {
        if (!_isMounted.current || !api.current) return
        if (commit) {
            api.current.clearHistory()
        } else {
            api.current.restoreAll()
        }
    }, []);

    return useMemo(() => (
        <div className={"visualizer"}>
            <CBVisualizer
                material={state.materialProperties}
                defaultMaterial={state.showControls ? GRID_MATERIAL : DEFAULT_MATERIAL}
                toolMode={toolMode}
                showControls={state.showControls}
                scene={state.sceneData}
                onSceneLoaded={sceneLoaded}
                onSceneRender={sceneRendered}
                fov={fov}
                cameraPosition={position}
                cameraRotation={[rotation[0], 0, rotation[2]]}
                floorRotation={rotation[1] + (rotationControlActive ? rotationControlValue : (state.floorRotationOffset || 0))}
                canLoad={canLoad}
                onHistoryChanged={historyChanged}
                blendEdges={isUploadedImage()}
            />
            {!state.showControls && state.sceneData &&
                <VisualizerTools
                    visible={!isToolOverlayOpen}
                    onChangeImage={onChangeImage}

                    onRotationChanged={rotateChanged}
                    onRotationFinished={rotateFinished}

                    editAreaEnabled={canEditArea()}
                    onEditArea={editAreaClicked}
                    historySize={historySize}
                    drawingToolMode={toolMode}
                    onDrawingToolModeChanged={drawingToolChanged}
                    onUndoClicked={drawingToolUndo}
                    onDrawingFinished={drawingFinished}

                    onShowHideButtons={toolsShowHideButtons}
                />}
            <ImageUpload onImageChosen={onImageChosen}/>
        </div>
    ), [state.materialProperties, state.showControls, state.sceneData, state.floorRotationOffset,
        toolMode, sceneLoaded, sceneRendered, fov, position, rotation, rotationControlActive, rotationControlValue,
        canLoad, historyChanged, isUploadedImage, isToolOverlayOpen, onChangeImage, rotateChanged, rotateFinished, canEditArea,
        editAreaClicked, historySize, drawingToolChanged, drawingToolUndo, drawingFinished, toolsShowHideButtons, onImageChosen])
}
