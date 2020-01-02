import {CBMaterialProperties} from "react-home-harmony";

export enum MediaPaths {
    Shared = "assets/img/shared",
    Scenes = "assets/scenes",
    Brands = "brands",
    Textures = "assets/textures",
}

export const MAX_IMAGE_SIZE = 2048;
export const DEFAULT_SCENE = "/bedroom/bedroom-3"

export const GRID_MATERIAL:CBMaterialProperties = {
    ppi:13,
    diffuseUrl:"assets/textures/grid.jpg"
}


export const DEFAULT_MATERIAL:CBMaterialProperties = {
    ppi:50,
    diffuseUrl:"assets/textures/cement/patchy_cement1_Base_Color.png",
    normalsUrl:"assets/textures/cement/patchy_cement1_Normal.png",
    specularUrl:"assets/textures/cement/patchy_cement1_Roughness.png"
}
