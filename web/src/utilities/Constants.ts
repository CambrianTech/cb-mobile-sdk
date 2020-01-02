import {CBMaterialProperties} from "react-home-harmony";

export enum MediaPaths {
    Shared = "assets/img/shared",
    Scenes = "assets/scenes",
    Brands = "brands",
}

export const MAX_IMAGE_SIZE = 2048;
export const DEFAULT_SCENE = "/kitchen/kitchen-2"

export const DEFAULT_MATERIAL:CBMaterialProperties = {
    ppi:50,
    diffuseUrl:"assets/textures/cement/patchy_cement1_Base_Color.png",
    normalsUrl:"assets/textures/cement/patchy_cement1_Normal.png",
    specularUrl:"assets/textures/cement/patchy_cement1_Roughness.png"
}
