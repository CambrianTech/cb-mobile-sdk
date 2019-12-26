import React, {ReactNode, useState, useRef, useEffect, useContext} from "react"
import { useDropzone } from "react-dropzone"
import {CBSceneParams, getRotatedFile, CBContentManager} from "react-home-harmony";
import {MediaPaths, SiteContext} from "../data/SiteContext";
import {safelyTimeout} from "react-home-harmony/dist/core/CBSceneUtils";
import {Progress} from "./Progress";
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

export function openFileDialog() {
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

    let _isMounted = useRef(false);

    useEffect(() => {
        _isMounted.current = true;

        return () => {
            _isMounted.current = false
        }
    }, []);

    async function upload(acceptedFiles: File[]) {
        setProgressPercentage(0);
        setStatusText("Your photo is being uploaded");

        const firstFile = acceptedFiles[0];

        if (!firstFile)
            return;

        const messageMinDurationMS = 4000
        const startTime = new Date();
        setProgressVisible(true);

        const maxWidth = 2048;
        const uploadFile = await getRotatedFile(firstFile, maxWidth);

        const firstFilePreviewPath = URL.createObjectURL(uploadFile);

        CBContentManager.default.resetScene()

        const results = await CBContentManager.default.uploadRoom(uploadFile, ((progress, status) => {
            setProgressPercentage(progress);
            setStatusText(status);
        })).catch((error)=>handleError(error));

        const roomId = CBContentManager.default.roomId

        if (results && roomId) {
            setStatusText("Ready");
            setProgressPercentage(1);

            const imageProps = {
                materialUrl: MediaPaths.DefaultMaterial,
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
            setStatusText("Upload failed")
        }

        const elapsed = new Date().getMilliseconds() - startTime.getMilliseconds()
        safelyTimeout(() => {
            if (_isMounted.current) {
                setProgressVisible(false)
            }
        }, Math.max(messageMinDurationMS - elapsed, 100))
    }

    function handleError(e: any) {

        safelyTimeout(() => {
            setProgressVisible(false)
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