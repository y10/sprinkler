function Slider(el) {
    var slider = this;

    var sliderWindow = el;

    var slidesElement = el.querySelector(".slides");

    var slidesNavElement = el.querySelector(".slides-nav");

    this.windowWidth = function () {
        var docElemProp = window.document.documentElement.clientWidth, body = window.document.body;
        return window.document.compatMode === "CSS1Compat" && docElemProp || body && body.clientWidth || docElemProp;
    }

    this.Show = function () {
        if (typeof (slidesElement.selectedIndex) !== 'undefined') {
            activate(slidesElement.selectedIndex);
        }
        else if (slidesElement.childElementCount > 0) {
            activate(0);
        }
        sliderWindow.style.display = '';
    }

    this.Hide = function () {
        sliderWindow.style.display = 'none';
    }

    this.Slide = function (direction) {
        var selectedIndex = typeof (slidesElement.selectedIndex) === 'undefined' ? 0 : slidesElement.selectedIndex;

        if (direction == 'back') { selectedIndex--; }
        if (direction == 'forward') { selectedIndex++; }

        if (selectedIndex == -1) { this.GoTo(slidesElement.childElementCount - 1); }
        else if (selectedIndex == slidesElement.childElementCount) { this.GoTo(0); }
        else { this.GoTo(selectedIndex); }
    }

    this.GoTo = function (selectedIndex) {
        var $windowwidth = this.windowWidth();
        var $margin = $windowwidth * selectedIndex;

        if (typeof (slidesElement.selectedIndex) !== 'undefined') {
            deactivate(slidesElement.selectedIndex);
        }

        activate(selectedIndex);

        slidesElement.style.transform = 'translate3d(-' + $margin + 'px,0px,0px)';

    }

    this.attachSwipeHandlers = function (hammer) {

        function onSwipe(ev) {

            switch (ev.type) {
                case "swipeleft":
                    slider.Slide('forward');
                    break;

                case "swiperight":
                    slider.Slide('back');
                    break;
            }
        }

        new Hammer(slidesElement).on("swipeleft swiperight", onSwipe);
    }

    function activate(selectedIndex) {
        var slideElement = slidesElement.children[selectedIndex];

        slideElement.classList.add('alive');

        if (typeof slideElement.activate !== "undefined") {
            slideElement.activate();
        }

        if (slidesNavElement) {
            var slideNavElement = slidesNavElement.children[selectedIndex];

            slideNavElement.classList.add("selected")
        }

        slidesElement.selectedIndex = selectedIndex;
    }

    function deactivate(selectedIndex) {
        var slideElement = slidesElement.children[selectedIndex];

        slideElement.classList.remove('alive');

        if (typeof slideElement.deactivate !== "undefined") {

            slideElement.deactivate();
        }

        if (slidesNavElement) {
            var slideNavElement = slidesNavElement.children[selectedIndex];

            slideNavElement.classList.remove("selected")
        }
    }

    function resize() {
        var WindowWidth = slider.windowWidth();
        slidesElement.style.width = (slidesElement.childElementCount * WindowWidth) + 'px';
        for (var i = 0; i < slidesElement.childElementCount; i++) {
            var li = slidesElement.children[i];
            li.style.width = WindowWidth + 'px';
        }
    }

    function instrument() {

        var WindowWidth = slider.windowWidth();
        slidesElement.style.width = (slidesElement.childElementCount * WindowWidth) + 'px';
        for (var i = 0; i < slidesElement.childElementCount; i++) {
            var li = slidesElement.children[i];
            if (li.hasAttribute('onload')) {
                var fnText = li.getAttribute('onload');
                fnText = fnText.replace("this", "self");
                var fnRef = Function('self', fnText);
                fnRef(li);
            }
            li.classList.add('slide');
            li.style.width = WindowWidth + 'px';
        }

        if (slidesNavElement) {
            while (slidesNavElement.firstChild) {
                slidesNavElement.removeChild(slidesNavElement.firstChild);
            }

            for (var i = 0; i < slidesElement.childElementCount; i++) {
                var dot = document.createElement('span');
                dot.innerHTML = "&#9679;"
                dot.index = i;
                dot.onclick = function (e) { slider.GoTo(this.index); }
                slidesNavElement.appendChild(dot);
            }
        }
    }

    instrument();

    if (slidesElement.childElementCount > 0 && sliderWindow.style.display != 'none') {
        activate(0);
    }

    window.addEventListener("orientationchange", resize);

    window.addEventListener("resize", resize);
}