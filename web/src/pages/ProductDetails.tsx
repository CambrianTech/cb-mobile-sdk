import React, {useCallback, useContext, useEffect, useRef, useState} from 'react'
import './ProductDetails.css'
import {CBVisualizer} from "react-home-harmony";
import {SiteContext} from "../data/SiteContext";
import {selectScene} from "../utilities/Methods";
import {DEFAULT_MATERIAL, DEFAULT_SCENE} from "../utilities/Constants";
import {api} from "../index";

type DetailsProps = {
    name?: string
    category?: any
    product?: any
    color?: any,
    fields?: any
}

const demoDetails:DetailsProps = {
    name: "adf",
    category: {
        "name":"hardwood",
        "displayName":"Hardwood",
        "thumbnailPath":"bundle/swatches/hardwood.jpg",
        "source":"odata/Hardwoods",
        "select":"CollectionDesc,ColorFamilyDesc,CollectionDesc",
        "productsQuery": "(StaticRoomFlag eq true or HasRenderImage eq true) and (IsDefaultStyleColor eq true)",
        "colorsQuery": "(StaticRoomFlag eq true or HasRenderImage eq true)",
        "fields": {
            "SellingStyleName":"Style Name",
            "SellingStyleNbr":"Style Number",
            "SellingColorName":"Color Name",
            "SellingColorNbr":"Color Number",
            "CollectionDesc":"Collection"
        }
    },
    product: {

    },
    color: {
        "SellingStyleName":"Big Product",
        "SellingColorName":"Special",
        "SellingStyleNbr":"ASDF 1235",
        "ColorFamilyDesc":"Beige/Tan",
        "CollectionDesc":"Test things"
    }
}

const ProductInfo = React.memo<DetailsProps>(
    (cProps) => {
        if (cProps.name) {
            return (<div className={"product-info-details"}>

                    <div className="product-info-content">
                        <h2>{cProps.color["SellingColorName"]}</h2>
                        <h3>#{cProps.color["SellingStyleNbr"]}</h3>

                        <ul className="product-info-properties">
                            <li key={"flooring-type"}>
                                <h4>Flooring Type</h4>
                                <p>{cProps.category["displayName"]}</p>
                            </li>
                            {cProps.category.fields && Object.keys(cProps.category.fields).map((key) => {
                                const displayName = cProps.category.fields[key]
                                const value = cProps.color[key]
                                if (value) {
                                    return <li key={key}>
                                        <h4>{displayName}</h4>
                                        <p>{value}</p>
                                    </li>
                                }
                                return null
                            })}
                        </ul>
                    </div>
                </div>
            );
        }
        return null;
    },
    (prevProps, nextProps) => {
        return  prevProps.name === nextProps.name;
    }
);

export default function ProductDetails(props: any) {

    const siteContext = useContext(SiteContext)!;
    const state = siteContext.state;
    const dispatch = siteContext.dispatch

    const position = state.position || [0, 1, 0];
    const rotation = state.rotation || [0, 0, 0];
    const fov = state.fov || 60;

    const [details, setDetails] = useState<DetailsProps>(process.env.NODE_ENV === 'development' ? demoDetails : {})

    const initialize = useCallback(() => {
        selectScene(DEFAULT_SCENE, dispatch)
        if (api.notifyLoaded) {
            api.notifyLoaded("ProductDetails")
        }
    }, [dispatch])

    const initializeRef = useRef(initialize);
    useEffect(() => { initializeRef.current = initialize; }, [initialize]);

    useEffect(() => {
        if (initializeRef.current) {
            initializeRef.current()
        }
    }, []);

    (window as any).cb.setProductDetails = useCallback((props:DetailsProps) => {
        setDetails(props)
    }, [])

    useEffect(() => {
        if (api.onSceneLoad && state.sceneData) {
            api.onSceneLoad()
        }
    }, [state.sceneData]);

    return (
        <div className="product-details">
            <div className={"product-visualizer"}>
                <CBVisualizer
                    material={state.materialProperties}
                    defaultMaterial={DEFAULT_MATERIAL}
                    scene={state.sceneData}
                    fov={fov}
                    cameraPosition={position}
                    cameraRotation={[rotation[0], 0, rotation[2]]}
                    floorRotation={rotation[1]}
                />
            </div>

            <ProductInfo {...details} />
        </div>
    )
}

