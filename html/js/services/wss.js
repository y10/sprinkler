Wss = (function () {

    var onSuccess = [];
    var onError = [];

    var websock = null;

    return {

        on: function (onSuccessCallback, onErrorCallback) {

            if (!websock && window.location.hostname)
            {
                websock = new WebSocket('ws://' + window.location.hostname + ':80/ws');
                websock.onopen = function (evt) { console.log('WS: open'); };
                websock.onclose = function (evt) { console.log('WS: close'); };
            
                websock.onerror = function (evt) {
                    console.log("WS: error");
                    console.log(evt);
                    
                    onError.forEach(function (callback) {
                        callback(evt);
                    }, this);
                };
            
                websock.onmessage = function (evt) {
                    console.log(evt);
            
                    var state = JSON.parse(evt.data);
                    onSuccess.forEach(function (callback) {
                        callback(state);
                    }, this);
                };
            }

            if (onSuccessCallback) {
                onSuccess.push(onSuccessCallback);
            }

            if (onErrorCallback) {
                onError.push(onErrorCallback);
            }
        }
    }
})();
