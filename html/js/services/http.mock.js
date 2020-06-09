Http = (function () {
    return {

        get: function (service, onSuccess, onError) {
            setTimeout(function(){console.log("GET " + service + " SUCCESSEDED"); onSuccess({})}, 1000);
        },

        post: function (service, json, onSuccess, onError) {
            setTimeout(function(){console.log("POST " + service + " " + JSON.stringify(json)); onSuccess({})}, 1000);
        },

        import: function (file, async, onLoad) {

            var scriptTag = document.createElement("script");
            scriptTag.src = file;
            scriptTag.async = async;
            scriptTag.onload = onLoad;

            if (async) {
                setTimeout(function () { document.body.appendChild(scriptTag); }, 10);
            }
            else {
                document.body.appendChild(scriptTag);
            }
        },
    }
})();
