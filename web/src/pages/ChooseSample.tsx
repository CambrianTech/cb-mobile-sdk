import React, {useCallback, useContext, useEffect, useRef} from 'react'
import './ChooseSample.css'
import {SiteContext} from "../data/SiteContext";

type ChooseSampleProperties = {

}

export function ChooseSample(props: ChooseSampleProperties) {

    const siteContext = useContext(SiteContext)!;
    const state = siteContext.state;
    const dispatch = siteContext.dispatch

    const initialize = useCallback(() => {

    }, [])

    const initializeRef = useRef(initialize);
    useEffect(() => { initializeRef.current = initialize; }, [initialize]);

    useEffect(() => {
        if (initializeRef.current) {
            initializeRef.current()
        }
    }, []);

    return (
        <div className="choose-sample">
            Samples
        </div>
    )
}