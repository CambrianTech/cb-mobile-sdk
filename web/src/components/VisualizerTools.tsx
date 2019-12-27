import React, {useCallback} from 'react'
import './VisualizerTools.css'
import MaterialIcon from "@material/react-material-icon";
import {Fab} from "@material/react-fab";

type VisualizerToolsProperties = {
    onChangeImage:()=>void
}

export function VisualizerTools(props: VisualizerToolsProperties) {

    const onChangeImage = useCallback((e:any) => {
        props.onChangeImage()
    }, [props])

    const onRotate = useCallback(() => {

    }, [])

    const onEdit = useCallback(() => {

    }, [])

    return (
        <div className="visualizer-tools">
            <Fab className="tool-button" onClick={onChangeImage} icon={<MaterialIcon icon='add_a_photo' />} />
            <Fab className="tool-button" onClick={onRotate} icon={<MaterialIcon icon='rotate_right' />} />
            <Fab className="tool-button" onClick={onEdit} icon={<MaterialIcon icon='edit' />} />
        </div>
    )
}