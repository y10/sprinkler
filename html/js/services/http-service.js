Http = (function () {
    return {

        get: function (service, onSuccess, OnError) {
            var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function () {
                if (this.readyState == 4 && this.status == 200) {
                    onSuccess(JSON.parse(this.responseText));
                }
            };
            xhttp.open('GET', service, true);
            xhttp.send();
        },

        getScript: function (file, async, onLoad) {

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

        getXml: function (file, onLoad) {
            var loadXml = new XMLHttpRequest;
            loadXml.onload = function (e) {
                onLoad(new DOMParser().parseFromString(loadXml.responseText, "text/xml").documentElement);
            };
            loadXml.open("GET", file, true);
            loadXml.send();
        }
    }
})();
