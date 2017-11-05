if (typeof app === "undefined") { app = {} }
if (typeof app.modules === "undefined"){ app.modules = {} }

app.modules.timer = (function (http) {
    
    return {
        load: function (el) {
            function createButton() {
                var btn = document.createElement('button');

                btn.redraw = function (state) {
                    btn.innerText = state.timer
                        ? Math.floor(state.timer / 60000) + ":" + ("0" + Math.floor((state.timer % 60000) / 1000)).slice(-2)
                        : '15:00';
                };

                btn.onclick = function () {

                    btn.disabled = true;

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
                            http.get('/api/start?t=15', function (result) {
                                btn.tick();
                                btn.disabled = false;
                            });
                        }
                    });
                }

                btn.tick = function () {
                    http.get('/api/state', function (state) {
                        btn.redraw(state);
                        if (state.timer) {
                            if (!btn.disabled) {
                                setTimeout(function () {
                                    btn.tick();
                                }, 2500)
                            }
                        }
                    });
                }

                btn.redraw({});

                return btn;
            }

            var div = document.createElement('section');
            div.className = "slide-container";
            el.appendChild(div);

            var btn = createButton();
            el.activate = function () {
                http.get('/api/state', function (state) {
                    btn.redraw(state);
                    btn.disabled = false;
                    if (state.timer) {
                        btn.tick();
                    }
                });
            }

            el.deactivate = function () {
                btn.disabled = true;
            }

            div.appendChild(btn);
        }
    };
})(Http);