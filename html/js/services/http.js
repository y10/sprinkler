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

        post: function (service, json, onSuccess, onError) {
            var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function () {
                if (this.readyState == 4)
                {
                    if (this.status == 200 && typeof onSuccess !== "undefined") {
                        onSuccess(JSON.parse(this.responseText));
                    } else if(typeof onError !== "undefined") {
                        onError(this.response);
                    } else {
                        onSuccess(this.response);
                    }
                }
            };

            xhttp.open('POST', service, true);
            xhttp.setRequestHeader("Content-Type", "application/json;charset=UTF-8");
            xhttp.send(JSON.stringify(json));
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
