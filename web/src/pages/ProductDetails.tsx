import React, {useCallback, useContext, useEffect, useRef, useState} from 'react'
import './ProductDetails.css'
import {CBVisualizer} from "react-home-harmony";
import {SiteContext} from "../data/SiteContext";
import {selectScene} from "../utilities/Methods";
import {DEFAULT_MATERIAL, DEFAULT_SCENE} from "../utilities/Constants";

type DetailsProps = {
    name?: string
    category?: string
    product?: any
    color?: any
}

const testDetails:DetailsProps = {
    name: "adf",
    category: "Hardwood",
    product: {

    },
    color: {
        "SellingColorName":"Test",
        "SellingStyleNbr":"ASDF 1235",
        "ColorFamilyDesc":"Beige/Tan",
        "CollectionDesc":"Test things"
    }
}

type SpecField = {
    name:string
    displayName:string
}

const spec_fields:Array<SpecField> = [
    {
        name:"CollectionDesc",
        displayName:"Collection"
    },
    {
        name:"ColorFamilyDesc",
        displayName:"Color Family"
    }
]

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
                                <p>{cProps.category}</p>
                            </li>
                            {spec_fields.map((field) => {
                                const value = cProps.color[field.name]
                                return <li key={field.name}>
                                    <h4>{field.displayName}</h4>
                                    <p>{value}</p>
                                </li>
                            })}
                        </ul>
                    </div>
                </div>
            );
        }
        return (<aside />);
    },
    (prevProps, nextProps) => {
        return  prevProps.name === nextProps.name;
    }
);

type ProductDetailsProperties = {

}

export function ProductDetails(props: ProductDetailsProperties) {

    const siteContext = useContext(SiteContext)!;
    const state = siteContext.state;
    const dispatch = siteContext.dispatch

    const position = state.position || [0, 1, 0];
    const rotation = state.rotation || [0, 0, 0];
    const fov = state.fov || 60;

    const [details, setDetails] = useState<DetailsProps>(testDetails)

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

    (window as any).cb.setProductDetails = useCallback((props:DetailsProps) => {
        alert(props)
        setDetails(props)
    }, [])

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

