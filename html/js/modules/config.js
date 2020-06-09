if (typeof app.modules === "undefined") { app.modules = {} }

app.modules.config = (function (http) {

    function createButton(key, name, onclick) {

        var button = document.createElement("button");
        button.id = key;
        button.innerText = name;
        button.onclick = onclick;

        var div = document.createElement("div");
        div.style.cssText = "padding:5px"
        div.appendChild(button);

        return div;
    }

    
    function createSetting(key, type, placeholder, onchange) {

        var input = document.createElement("input");
        input.id = key;
        input.type = type;
        input.placeholder = placeholder;

        if (onchange) input.onchange = function() {
            onchange(input.value);
        }

        var div = document.createElement("div");
        div.render = function (settings) {
            if (settings && typeof settings[key] !== "undefined") {
                input.value = (type == "password") ? window.atob(settings[key]) : settings[key];
                input.defaultValue = input.value;
            }
        };
        div.save = function (settings) {
            if (input.value !== input.defaultValue){
                settings[key] = input.value;
            }
        };
        div.appendChild(input);

        return div;
    }

    return {
        load: function (element) { 

            var elements = 
            [
                createSetting("disp_name", "text", "display name", function (value) {
                    Http.post("/api/settings", { "disp_name": value });
                    Reload(5000);
                }),
                createButton("restart", "Restart", function () {
                    http.get('/restart');
                    Reload(5000);
                }),
                createButton("update", "Update", function () {
                    http.get("api/settings", function (settings) {
                        var url = prompt("Are you sure you want to update from?", settings['upds_addr'] || "http://ota.voights.net/sprinkler.bin");
                        if (url != null) {
                            Http.post("/update", { "upds_addr": url }, function () {
                                Reload(30000);
                            });
                        }
                    });
                }),
                createButton("reset", "Reset", function () {
                    if (confirm("Are you sure you want to continue?")) {
                        http.get('/reset');
                        Reload(5000);
                    }
                })
            ]

            var section = document.createElement('section');
            section.className = "slide-container";
           
            elements.forEach(function (el) {
                section.appendChild(el);
            }, this);

            element.render = function (data) {
                elements.forEach(function (el) {
                    if(el["render"]){
                        el.render(data);
                    }
                }, this);
            };
    
            element.activate = function () {
                http.get('/api/settings', function (result) {
                    element.render(result);
                });
            };

            element.appendChild(section);
        }
    };

})(Http);