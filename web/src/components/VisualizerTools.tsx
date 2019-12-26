import React, {ButtonHTMLAttributes, ReactNode} from 'react'
import 'react-circular-progressbar/dist/styles.css'
import './VisualizerTools.css'

type ToolButtonProperties = {

} & ButtonHTMLAttributes<HTMLButtonElement>

function ToolButton(props: ToolButtonProperties) {
    return (
        <button className="tool-button" {...props}>
            {props.children}
        </button>
    )
}

type VisualizerToolsProperties = {
    onChangeImage:()=>void
}

export function VisualizerTools(props: VisualizerToolsProperties) {
    return (
        <div className="visualizer-tools">
            <ToolButton title={"Change Image"} onClick={props.onChangeImage}>
                Change Image
            </ToolButton>
        </div>
    )
}