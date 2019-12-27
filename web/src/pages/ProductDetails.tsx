import React, {useCallback, useContext, useEffect, useMemo, useRef} from 'react'
import './ProductDetails.css'
import {CBMaterialProperties, CBVisualizer} from "react-home-harmony";
import {SiteContext} from "../data/SiteContext";
import {selectScene} from "../utilities/Methods";
import {DEFAULT_SCENE} from "../utilities/Constants";

type ProductDetailsProperties = {

}

export function ProductDetails(props: ProductDetailsProperties) {

    const siteContext = useContext(SiteContext)!;
    const state = siteContext.state;
    const dispatch = siteContext.dispatch

    const position = state.position || [0, 1, 0];
    const rotation = state.rotation || [0, 0, 0];
    const fov = state.fov || 60;

    const defaultMaterial = useMemo<CBMaterialProperties>(() => {
        return {
            ppi:20,
            diffuseUrl:"assets/textures/floor/narrow-floorboards1-albedo.png",
            normalsUrl:"assets/textures/floor/narrow-floorboards1-normal-dx.png",
            specularUrl:"assets/textures/floor/narrow-floorboards1-roughness.png"
        }
    }, [])

    const initialize = useCallback(() => {
        selectScene(DEFAULT_SCENE, dispatch)
    }, [dispatch])

    const initializeRef = useRef(initialize);
    useEffect(() => { initializeRef.current = initialize; }, [initialize]);

    useEffect(() => {
        if (initializeRef.current) {
            initializeRef.current()
        }
    }, []);

    return (
        <div className="product-details">
            <CBVisualizer
                material={state.materialProperties}
                defaultMaterial={defaultMaterial}
                scene={state.sceneData}
                fov={fov}
                cameraPosition={position}
                cameraRotation={[rotation[0], 0, rotation[2]]}
                floorRotation={rotation[1]}
            />
        </div>
    )
}