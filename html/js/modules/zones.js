if (typeof app === "undefined") { app = {} }
if (typeof app.modules === "undefined"){ app.modules = {} }

app.modules.zones = (function (http) {
    
    return {
        load: function (el, svg)  {

            function createButton() {
                var btn = document.createElement('button');
               
                function createIcon() {
                    var icon = null;

                    if (svg) {
                        icon = svg.cloneNode(true);
                        icon.setAttribute("width", "150");
                        icon.setAttribute("height", "150");
                    }
                    else {
                        icon = document.createElement('img');
                        icon.src = '/icon.png'
                        icon.width = '150'
                        icon.height = '150'
                    }

                    return icon;
                }

                btn.redraw = function (state) {

                    while (btn.firstChild) {
                        btn.removeChild(btn.firstChild);
                    }

                    for (var i = 0; i < (state.zones ? state.zones : 5); i++) {
                        var icon = createIcon();
                        icon = btn.appendChild(icon);
                        icon.style.opacity = 0.1;
                    }

                    if (btn.lastChild && state.timer) {
                        btn.lastChild.setAttribute('fill', 'orange');
                        btn.lastChild.style.opacity = state.timer / (15 * 60000);
                    }
                }

                btn.onclick = function () {

                    http.get('/api/state', function (state) {

                        if (state.started) {

                            if (state.paused) {
                                http.get('/api/resume', function (result) {
                                    btn.tick();
                                    btn.disabled = false;
                                });
                            } else {
                                http.get('/api/pause', function (result) {
                                    btn.disabled = false;
                                });
                            }
                        } else {
                            http.get('/api/start?t=15&z=5', function (result) {
                                pages.Slide('back');
                            });
                        }
                    });
                    btn.disabled = true;
                }

                btn.tick = function () {
                    http.get('/api/state', function (state) {
                        btn.redraw(state);
                        if (!btn.disabled) {
                            setTimeout(function () {
                                btn.tick();
                            }, 2500)
                        }
                    });
                }

                btn.redraw({});

                return btn;
            }

            var div = document.createElement('div');
            div.className = "slide-container";

            var btn = null;

            el.activate = function () {

                if (!btn) {
                    btn = createButton();
                    div.appendChild(btn);
                }

                http.get('/api/state', function (state) {
                    btn.disabled = false;

                    if (state.zones) {

                        btn.tick();
                    }
                });
            }

            el.deactivate = function () {
                btn.disabled = true;
            }

            el.appendChild(div);
        }
    };
})(Http);