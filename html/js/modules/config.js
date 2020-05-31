if (typeof app.modules === "undefined") { app.modules = {} }

app.modules.config = (function (http) {

    function createMenu(key, name, onclick) {

        var button = document.createElement("button");
        button.id = key;
        button.innerText = name;
        button.onclick = onclick;

        var div = document.createElement("div");
        div.style.cssText = "padding:5px"
        div.appendChild(button);

        return div;
    }

    return {
        load: function (element) { 
            var section = document.createElement('section');
            section.className = "slide-container";

            [
                createMenu("restart", "Restart", function () {
                    http.get('/restart');
                    Reload(5000);
                }),
                createMenu("update", "Update", function () {
                    http.get("api/settings", function (settings) {
                        var url = prompt("Are you sure you want to update from?", settings['upds_addr'] || "http://ota.voights.net/sprinkler.bin");
                        if (url != null) {
                            Http.post("/update", { "upds_addr": url }, function (response) {
                                Reload(30000);
                            });
                        }
                    });
                }),
                createMenu("reset", "Reset", function () {
                    if (confirm("Are you sure you want to continue?")) {
                        http.get('/restart');
                        Reload(5000);
                    }
                })
            ]
            .forEach(function (el) {
                section.appendChild(el);
            }, this);

            element.appendChild(section);
        }
    };

})(Http);