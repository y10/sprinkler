if (typeof app === "undefined") { app = {} }
if (typeof app.modules === "undefined"){ app.modules = {} }


app.modules.main = (function (http, wss) {
    
    return {
        load: function (el) {

            function createButton() {

                var label = document.createElement("label");
                label.className = "switch";

                var input = document.createElement("input");
                input.type = "checkbox";
                label.appendChild(input)

                var span = document.createElement("span");
                span.className = "slider round";
                label.appendChild(span)

                label.redraw = function (state) {
                    input.checked = state.started ? true : false;
                }
                label.onclick = function (e) {
                    e.preventDefault();
                    label.disabled = true;
                    http.get('/api/' + (input.checked ? 'stop' : 'start'), function (result) {
                        label.redraw(result);
                        label.disabled = false;
                    });
                }

                wss.on(function(state){

                    label.redraw(state);
                    
                });

                return label;
            }

            el.className = 'slide color-0 alive';
            var btn = createButton();

            el.activate = function () {
                http.get('/api/state', function (state) {
                    btn.redraw(state);
                    btn.disabled = false;
                });
            }

            el.deactivate = function () {
                btn.disabled = true;
            }

            var div = document.createElement('div');
            div.className = "slide-container";
            el.appendChild(div);
            div.appendChild(btn);
        }
    };

})(Http, Wss);