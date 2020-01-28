import React, {useCallback, useEffect, useRef} from 'react'
import './BrandInfo.css'
import {MediaPaths} from "../utilities/Constants";
import {api} from "../index";

export default function BrandInfo(props: any) {

    const initialize = useCallback(() => {
        if (api.notifyLoaded) {
            api.notifyLoaded("BrandInfo")
        }
    }, [])

    const initializeRef = useRef(initialize);
    useEffect(() => { initializeRef.current = initialize; }, [initialize]);

    useEffect(() => {
        if (initializeRef.current) {
            initializeRef.current()
        }
    }, []);

    const brandPath = MediaPaths.Brands + "/shawfloors"

    return (
        <div className="brand-info">
            <img className={"splash"} src={brandPath + "/splash.png"} alt={"Splash"} />
            <img className={"logo"} src={brandPath + "/logo.png"} alt={"Logo"}
                 srcSet={`${brandPath}/logo.png, ${brandPath}/logo@2x.png 2x, ${brandPath}/logo@3x.png 3x`} />
        </div>
    )
}