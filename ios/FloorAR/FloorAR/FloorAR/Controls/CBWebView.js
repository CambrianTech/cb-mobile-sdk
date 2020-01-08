if (window.attachEvent) {window.attachEvent('onload', webviewLoaded);}
else if (window.addEventListener) {window.addEventListener('load', webviewLoaded, false);}
else {document.addEventListener('load', webviewLoaded, false);}

function webviewLoaded() {
    window.cb.showHideProgress = showHideProgress;
    window.cb.setProgress = setProgress;
    window.cb.onSceneLoad = onSceneLoad;
    window.cb.notifyLoaded = notifyLoaded;
    window.cb.openImageDialog = openImageDialog;
    
    webkit.messageHandlers.callbackHandler.postMessage({'command':'loaded'})
}

function notifyLoaded(component) {
    webkit.messageHandlers.callbackHandler.postMessage({'command':'notifyLoaded','component':component})
}

function showHideProgress(show) {
    webkit.messageHandlers.callbackHandler.postMessage({'command':'showProgress', 'show':show})
}

function setProgress(progress, message) {
    webkit.messageHandlers.callbackHandler.postMessage({'command':'setProgress', 'progress':progress, 'message':message})
}

function onSceneLoad() {
    webkit.messageHandlers.callbackHandler.postMessage({'command':'sceneLoaded'})
}

function openImageDialog() {
    webkit.messageHandlers.callbackHandler.postMessage({'command':'openImageDialog'})
}
