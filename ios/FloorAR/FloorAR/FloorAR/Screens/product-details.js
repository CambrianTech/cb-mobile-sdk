
function webviewLoaded() {
    window.setTimeout(function() {
        webkit.messageHandlers.callbackHandler.postMessage({'command':'loaded'})
    }, 500)
}

if (window.attachEvent) {window.attachEvent('onload', webviewLoaded);}
else if (window.addEventListener) {window.addEventListener('load', webviewLoaded, false);}
else {document.addEventListener('load', webviewLoaded, false);}

function setProductDetails(materialJson, productJson) {
    window.cb.setMaterial(materialJson);
    window.cb.setProductDetails(productJson);
}
