import React, {ReactNode, useState, useRef, useEffect, useContext, useCallback} from "react"
import { useDropzone } from "react-dropzone"
import {CBSceneParams, getRotatedFile, CBContentManager} from "react-home-harmony";
import {SiteContext} from "../data/SiteContext";
import {Progress} from "./Progress";
import {safelyTimeout} from "../utilities/Methods";
import {MAX_IMAGE_SIZE} from "../utilities/Constants";
const fileAccept = "image/*";

export type ImageProperties = CBSceneParams & {
    roomId: string | null,
}

interface ImageUploadProperties {
    className?: string
    onImageChosen: (props: ImageProperties) => void
    children?: ReactNode
    ref?: any,
}



export function openImageDialog() {
    const inputs = Array.from(document.getElementsByTagName("input"));
    const input = inputs.find(element => element.type === "file") as HTMLInputElement;

    if (input) {
        input.click()
    }
}

export function ImageUpload(props: ImageUploadProperties) {
    const siteContext = useContext(SiteContext)!
    const [statusText, setStatusText] = useState("")
    const [progressPercentage, setProgressPercentage] = useState(0)
    const [progressVisible, setProgressVisible] = useState(false)
    const api:any = (window as any).cb
    api.openImageDialog = openImageDialog

    let _isMounted = useRef(false);

    useEffect(() => {
        _isMounted.current = true;

        return () => {
            _isMounted.current = false
        }
    }, []);

    const showHideProgress = useCallback((visible:boolean) => {
        if (api.showHideProgress) {
            api.showHideProgress(visible)
        } else {
            setProgressVisible(visible)
        }
    }, [])

    const setProgress = useCallback((progress:number, message:string) => {
        if (api.setProgress) {
            api.setProgress(progress, message)
        } else {
            setProgressPercentage(progress);
            setStatusText(message);
        }
    }, [])

    const dataURItoBlob = function(dataURI: string) {
        // convert base64/URLEncoded data component to raw binary data held in a string
        let byteString
        if (dataURI.split(',')[0].indexOf('base64') >= 0)
            byteString = atob(dataURI.split(',')[1])
        else
            byteString = unescape(dataURI.split(',')[1])

        // separate out the mime component
        const mimeString = dataURI.split(',')[0].split(':')[1].split(';')[0]

        // write the bytes of the string to a typed array
        const ia = new Uint8Array(byteString.length)
        for (let i = 0; i < byteString.length; i++) {
            ia[i] = byteString.charCodeAt(i)
        }

        return new Blob([ia], { type: mimeString })
    }
    
    api.uploadPhotoData = function(data:string) {
        let blob = dataURItoBlob(data)
        let file = new File([blob], "")
        upload([file])
    }

    async function upload(acceptedFiles: File[]) {
        const firstFile = acceptedFiles[0];

        if (!firstFile)
            return;

        const messageMinDurationMS = 4000
        const startTime = new Date();
        setProgress(0, "Your photo is being uploaded")
        showHideProgress(true)

        const uploadFile = await getRotatedFile(firstFile, MAX_IMAGE_SIZE);

        const firstFilePreviewPath = URL.createObjectURL(uploadFile);

        CBContentManager.default.resetScene()

        const results = await CBContentManager.default.uploadRoom(uploadFile, ((progress, status) => {
            setProgress(progress, status)
        })).catch((error)=>handleError(error));

        const roomId = CBContentManager.default.roomId

        if (results && roomId) {
            setProgress(1, "Ready")

            const imageProps = {
                backgroundUrl: firstFilePreviewPath,
                lightingUrl: results.lightingUrl,
                dataUrl: results.dataUrl,
                superpixelsUrl: results.superpixelsUrl,
                masks: {
                    "floor": results.semanticUrl
                },
                roomId: roomId
            }

            if (results.dataUrl != null) {
                fetch(results.dataUrl).then(res => res.json()).then(data => {
                    props.onImageChosen({
                        scene: imageProps,
                        fov: data.fieldOfView,
                        cameraRotation: data.cameraRotation,
                        floorRotation: data.floorRotation,
                        cameraPosition: data.cameraPosition,
                        roomId: roomId
                    })
                })
            } else {

                props.onImageChosen({
                    scene: imageProps,
                    fov: results.fieldOfView,
                    cameraRotation: results.cameraRotation,
                    floorRotation: results.floorRotation,
                    cameraPosition: [0, results.cameraElevation, 0],
                    roomId: roomId
                })

            }
        } else {
            setProgress(progressPercentage, "Upload failed")
        }

        const elapsed = new Date().getTime() - startTime.getTime()
        safelyTimeout(() => {
            if (_isMounted.current) {
                showHideProgress(false)
            }
        }, Math.max(messageMinDurationMS - elapsed, 100))
    }

    function handleError(e: any) {

        safelyTimeout(() => {

            showHideProgress(false)

            switch (e.constructor) {
                case Promise: {
                    const promise = e as Promise<any>;
                    promise.catch((error: any) => {
                        siteContext.dispatch({ type: "setError", error: error })
                    })
                    break;
                }
                default: {
                    siteContext.dispatch({ type: "setError", error: e })
                }
            }
        }, 3000);
    }

    const { getRootProps, getInputProps } = useDropzone({
        accept: fileAccept,
        onDrop: upload
    });

    return (
        <div ref={props.ref}>
            <div className={props.className} {...getRootProps()}>
                <input {...getInputProps()} />
                {props.children}
            </div>
            <Progress visible={progressVisible} percentage={progressPercentage} statusText={statusText} />
        </div>
    )
}