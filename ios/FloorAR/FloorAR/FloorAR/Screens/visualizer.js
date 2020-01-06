if (window.attachEvent) {window.attachEvent('onload', webviewLoaded);}
else if (window.addEventListener) {window.addEventListener('load', webviewLoaded, false);}
else {document.addEventListener('load', webviewLoaded, false);}

function webviewLoaded() {
    window.cb.showHideProgress = showHideProgress;
    window.cb.setProgress = setProgress;
}

function showHideProgress(show) {
    webkit.messageHandlers.callbackHandler.postMessage({'command':'showProgress', 'show':show})
}

function setProgress(progress, message) {
    webkit.messageHandlers.callbackHandler.postMessage({'command':'setProgress', 'progress':progress, 'message':message})
}
