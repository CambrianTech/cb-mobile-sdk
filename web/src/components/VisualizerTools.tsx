import React, {useCallback, useContext, useState} from 'react'
import './VisualizerTools.css'
import MaterialIcon from "@material/react-material-icon";
import {Fab} from "@material/react-fab";
import {VisualizerRotateTool} from "./VisualizerRotateTool";
import {SiteContext} from "../data/SiteContext";
import {CBToolMode} from "react-home-harmony";

type VisualizerToolsProperties = {
    visible:boolean
    onChangeImage:()=>void
    onShowHideButtons: (show: boolean) => void

    onRotationChanged: (radians: number) => void
    onRotationFinished: (commit: boolean, radians:number) => void

    // editAreaEnabled?:boolean
    // onEditArea: () => void
    //
    // drawingToolMode:CBToolMode
    // onDrawingToolModeChanged: (mode:CBToolMode) => void
    // onUndoClicked: () => void
    // onDrawingFinished: (commit: boolean) => void
}

export default function VisualizerTools(props: VisualizerToolsProperties) {

    const [isOpen, setIsOpen] = useState()
    const [showRotateTool, setShowRotateTool] = useState(false)
    const [showDrawingTools, setShowDrawingTools] = useState(false)
    const siteContext = useContext(SiteContext)!

    const onChangeImage = useCallback((e:any) => {
        props.onChangeImage()
    }, [props])

    const onShowHideButtons = props.onShowHideButtons
    const onRotationFinished = props.onRotationFinished

    const toggleTools = useCallback(() => {
        const open = !isOpen
        setIsOpen(open)
    } , [isOpen]);

    const rotateFinished = useCallback((finished: boolean, radians:number) => {
        setShowRotateTool(false)
        onShowHideButtons(true)
        onRotationFinished(finished, radians)
    } , [onShowHideButtons, onRotationFinished]);

    const rotateButtonClicked = useCallback(() => {
        toggleTools()
        const showRotate = !showRotateTool
        onShowHideButtons(!showRotate)
        setShowRotateTool(showRotate)
    } , [toggleTools, onShowHideButtons, showRotateTool]);

    // const onDrawingToolModeChanged = props.onDrawingToolModeChanged
    // const onEditArea = props.onEditArea
    // const editAreaClicked = useCallback(() => {
    //     toggleTools()
    //     onShowHideButtons(false)
    //     setShowDrawingTools(true)
    //     onEditArea()
    //     onDrawingToolModeChanged(CBToolMode.Draw)
    // } , [toggleTools, onShowHideButtons, onEditArea, onDrawingToolModeChanged]);
    //
    // const onDrawingFinished = props.onDrawingFinished
    // const drawingFinished = useCallback((finished: boolean) => {
    //     setShowDrawingTools(false)
    //
    //     onDrawingToolModeChanged(CBToolMode.Select)
    //     onShowHideButtons(true)
    //     onDrawingFinished(finished)
    // } , [onDrawingFinished, onDrawingToolModeChanged, onShowHideButtons]);

    return (
        <div className="visualizer-tools">

            {props.visible && <Fab className="tool-button" onClick={onChangeImage} icon={<MaterialIcon icon='add_a_photo' />} />}
            {props.visible && <Fab className="tool-button" onClick={rotateButtonClicked} icon={<MaterialIcon icon='rotate_right' />} />}
                {/*<Fab className="tool-button" onClick={editAreaClicked} icon={<MaterialIcon icon='edit' />} />*/}


            <VisualizerRotateTool visible={showRotateTool}
                                  onRotationFinished={rotateFinished}
                                  onRotationChanged={props.onRotationChanged}
                                  rotation={siteContext.state.floorRotationOffset || 0} />
        </div>
    )
}