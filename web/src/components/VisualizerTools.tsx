import React, {useCallback} from 'react'
import 'react-circular-progressbar/dist/styles.css'
import './VisualizerTools.css'
import '@material/react-button/dist/button.css';
import Button from "@material/react-button";
import MaterialIcon from "@material/react-material-icon";


type VisualizerToolsProperties = {
    onChangeImage:()=>void
}

export function VisualizerTools(props: VisualizerToolsProperties) {

    const onRotate = useCallback(() => {

    }, [])

    const onEdit = useCallback(() => {

    }, [])

    return (
        <div className="visualizer-tools">
            <Button className="tool-button" raised onClick={props.onChangeImage}>
                <MaterialIcon icon='add_a_photo' />
            </Button>
            <Button className="tool-button" raised onClick={onRotate}>
                <MaterialIcon icon='rotate_right' />
            </Button>
            <Button className="tool-button" raised onClick={onEdit}>
                <MaterialIcon icon='edit' />
            </Button>
        </div>
    )
}