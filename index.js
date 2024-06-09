const MAX_PWM = 4095;
const MAX_TONE = 40000; //4294967295;
const MAX_SERVO = 180;

const css_normal = "btn btn-light";
const css_busy = "btn btn-info";
const css_warning = "btn btn-warning";
const css_bad = "btn btn-danger";

var pin_set = [2];

var activity;
var mode;
var stepsize = 1;

var dataset = [];

var chart;

var chartData = {
    datasets: [

    ]
};

const AnalogWrite = "analogout";
const AnalogRead = "analogin";
const DigitalWrite = "digitalout";
const DigitalRead = "digitalin";
const Servo = "servo";
const Tone = "tone";

const PinMode = { AnalogWrite, AnalogRead, DigitalWrite, DigitalRead, Servo, Tone };

function addData(data) {
    var time = new Date();
    dataset.push(data.value);
    localStorage.setItem("dataset", dataset);
}

$(function () {
    const exampleModal = document.getElementById('exampleModal');

    if (exampleModal) {
        exampleModal.addEventListener('show.bs.modal', event => {
            // Button that triggered the modal
            const button = event.relatedTarget
            // Extract info from data-bs-* attributes
            const recipient = button.getAttribute('data-bs-whatever')
            // If necessary, you could initiate an Ajax request here
            // and then do the updating in a callback.

            // Update the modal's content.
            const modalTitle = exampleModal.querySelector('.modal-title')
            const modalBodyInput = exampleModal.querySelector('.modal-body input')

            modalTitle.textContent = `New message to ${recipient}`
            modalBodyInput.value = recipient
        })
    }

    var cached_pins = localStorage.getItem("pin_set");

    if (cached_pins != null && cached_pins !== "") {
        pin_set = JSON.parse(cached_pins);
    }

    stepsize = localStorage.getItem("stepsize") || "1";

    $("#stepsize").val(stepsize).change();

    activity = $("#activity");

    document
        .getElementById("refreshBtn")
        .addEventListener("click", GetPinValues_click);

    document
        .getElementById("addSliderBtn")
        .addEventListener("click", AddNewSlider_Click);

    document
        .getElementById("stepsize")
        .addEventListener("change", stepSize_changed);

    document
        .getElementById("activity")
        .addEventListener("click", activity_click);

    document
        .getElementById("resetBtn")
        .addEventListener("click", HardResetAllPins);

    RebuildPins();
});

function RebuildPins() {
    $(".slider-set").empty();
    pin_set.forEach(AddSliderContainer);
}

function AddNewSlider_Click(evt) {
    var max = 2;
    if (pin_set.length > 0) {
        max = Math.max(...pin_set) + 1;
    }
    if (evt.ctrlKey) {
        pin = max;
    } else {
        var pin = prompt("Enter Pin Number", `${max}`);
        if (!pin || pin == null || pin == "") {
            return;
        }
        pin = Number(pin);
    }

    if (isNaN(pin) || pin_set.includes(pin) || pin < 2 || pin > 24) {
        return;
    }
    pin_set.push(pin);
    AddSliderContainer(pin);
    localStorage.setItem("pin_set", JSON.stringify(pin_set));
}

function AddSliderContainer(pin) {
    var slideContainer = CreateSliderContainer(pin);
    BindContainer(pin, slideContainer);
    $(".slider-set").append(slideContainer);
}

/**
 * @description Bind UI fields for pin group
 * @param {*} i 
 * @param {*} obj 
 */
function BindContainer(i, obj) {
    var elem = $(obj);
    var pin = elem.attr("x-pin");

    elem.find("span.slider-details-pin").html(`PIN ${pin}`);

    var mode = elem.find("select.slider-mode");
    var slider = elem.find("input.slider-range");
    var valueSpan = elem.find("span.slider-details-val");

    mode.on("change", function () {
        console.log('mode', this.value);
        var max = MAX_PWM;
        switch (this.value) {
            case "tone":
                max = MAX_TONE;
                break;
            case "servo":
                max = MAX_SERVO;
                break;
        }

        slider.attr("max", max);
    });

    slider.on("change", function (evt) {
        console.log('slider', arguments);

        $(valueSpan)
            .removeClass(css_normal)
            .addClass(css_warning)
            .removeClass(css_busy)
            .removeClass(css_bad);

        console.log("mode", mode.val());

        PostPinValue(mode.val(), pin, this.value, valueSpan);
    });

    slider.on("input", function (evt) {
        $(valueSpan)
            .removeClass(css_normal)
            .removeClass(css_warning)
            .removeClass(css_bad)
            .addClass(css_busy)
            .html(this.value);
    });

    valueSpan.on("click", function (evt) {
        var value = prompt(`Set pin ${pin} value`, slider.val());
        if (value == null || value == "") {
            return;
        }

        value = Number(value);
        if (isNaN(value)) {
            return;
        }
        if (value < 0) {
            value = 0;
        }

        slider.val(value);
        slider.trigger("input");
        slider.trigger("change");
    });

    elem.find("button.slider-controls-stop").on("click", function (evt) {

        if (evt.ctrlKey) {
            //evt.preventDefault();
            $(".slider").val(0).trigger("input").change();

            return;
        }

        slider.val(0).trigger("input").change();
    });

    elem.find("button.slider-controls-remove").on("click", function (evt) {
        var indx = pin_set.indexOf(Number(pin));
        pin_set.splice(indx, 1);
        RebuildPins();
        localStorage.setItem("pin_set", JSON.stringify(pin_set));
    });


    var pulseValue = elem.find("input.pulse-input-value");
    var pulseTime = elem.find("input.pulse-input-time");
    var pulseBtn = elem.find("button.pulse-btn");

    pulseBtn.on("click", function(evt) {
        console.log(`pulse: ${pin} | ${pulseValue.val()} | ${pulseTime.val()}`);
        PostPulsePin(pin, pulseValue.val(), pulseTime.val(), valueSpan);

    });

    var incrementBtn = elem.find("button.slider-increment-plus");
    incrementBtn.on("click", function(evt) {
        console.log("add one");
        slider.val(Number(slider.val()) + 1).trigger("input").change();
    });

    var decrementBtn = elem.find("button.slider-increment-minus");
    decrementBtn.on("click", function(evt) {
        console.log("add one");
        slider.val(Number(slider.val()) - 1).trigger("input").change();
    });

}

var refreshInterval;

function GetPinValues_click(evt) {

    if (evt.altKey) {
        location.reload();
        return;
    }

    if (evt.ctrlKey) {
        var refreshRate = prompt("Enter refresh rate (millisecconds)");
        if (refreshRate == null || refreshRate == "") {
            clearInterval(refreshInterval);
            return;
        }
        var time = Number(refreshRate);
        if (isNaN(time) || time <= 0) {
            clearInterval(refreshInterval);
            return;
        }
        if (time < 100) {
            time = 100;
        }

        console.log("starting timeout", time);

        refreshInterval = setInterval(RefreshPinValues, time);

        return;
    }

    clearInterval(refreshInterval);

    RefreshPinValues();
}

function GetPinValues_rclick() {
    alert("right click");
}

function InActivity(message) {
    AddActivity(message, `<span class="act-in"><i class="bi bi-telephone-inbound"></i></span>`);
}

function OutActivity(message) {
    AddActivity(message, `<span class="act-out"><i class="bi bi-telephone-outbound"></i></span>`);
}

function errActivity(message) {
    AddActivity(message, `<span class="act-bug"><i class="bi bi-bug"></i></span>`);
}

function AddActivity(message, direction) {
    var d = new Date();
    if (typeof message === "object") {
        message = JSON.stringify(message);
    }
    var item = `<tr>
        <td>${d.getHours()}:${d.getMinutes()}:${d.getSeconds()}.${d.getMilliseconds()}</td>
        <td>${direction}</td>
        <td>${message}</td>
    </tr>`;

    activity.prepend($(item));
}

function stepSize_changed(evt) {
    if (!this.value || this.value < 1) {
        this.value = 1;
    }

    localStorage.setItem('stepsize', this.value);
    console.log("stepSize_changed", this.value);
    $("input.slider-range").attr("step", this.value);
}

function activity_click(evt) {
    if (evt.ctrlKey) {
        $("#activity").empty();
    }
}

//#region === API ===

function HardResetAllPins(evt) {
    var url = "/api";

    var data = { cmd: "reset" };

    OutActivity({ url, ...data }, true);

    $.post(url, data, function (response) {
        InActivity(response);
        RebuildPins();
    }).fail(function (jqxhr, textStatus, errorThrown) {
        errActivity(jqxhr.statusCode());
    }).always(function () {
    });
}

var updatePending = false;
function PostPinValue(mode, pin, value, valueSpan) {
    updatePending = true;

    var url = `/${mode}`;
    var data = { pin: pin, value: value };

    OutActivity({ url, ...data }, true);

    $.post(url, data, function (response) {
        if (valueSpan) {
            $(valueSpan)
                .addClass(css_normal)
                .removeClass(css_warning)
                .removeClass(css_bad)
                .remove(css_busy);
        }
        InActivity(response);
    }).fail(function (jqxhr, textStatus, errorThrown) {
        errActivity(jqxhr.statusCode());
        if (valueSpan) {
            $(valueSpan)
                .removeClass(css_normal)
                .removeClass(css_warning)
                .removeClass(css_busy)
                .addClass(css_bad);
        }
    }).always(function () {
        updatePending = false;
    });
}

function PostPulsePin(pin, value, time, valueSpan) {
    updatePending = true;

    var url = `/pulse`;
    var data = { pin, value, time };

    OutActivity({ url, ...data }, true);

    $.post(url, data, function (response) {

        setSliderValue(pin, 0);

        // if (valueSpan) {
        //     $(valueSpan)
        //         .addClass(css_normal)
        //         .removeClass(css_warning)
        //         .removeClass(css_bad)
        //         .remove(css_busy);
        // }
        InActivity(response);
    }).fail(function (jqxhr, textStatus, errorThrown) {
        errActivity(jqxhr.statusCode());
        if (valueSpan) {
            $(valueSpan)
                .removeClass(css_normal)
                .removeClass(css_warning)
                .removeClass(css_busy)
                .addClass(css_bad);
        }
    }).always(function () {
        updatePending = false;
    });
}



function RefreshPinValues() {
    var url = `/analogin`;
    var data = { pins: JSON.stringify(pin_set) };
    $.post(url, data, function (response) {
        response.forEach(function (part) {
            addData(part);
            setSliderValue(part.pin, part.value);

        });
    }).fail(function (jqxhr, textStatus, errorThrown) {
        errActivity(jqxhr.statusCode());
    });
}

//#endregion === API ===


/**
 * 
 * @param {*} pin 
 * @param {*} value 
 */
function setSliderValue(pin, value) {
    var container = $(`[x-pin="${pin}"]`);
    $(container.find("input.slider-range")).val(value);
    $(container.find("span.slider-details-val")).html(value);
}


/**
 * 
 * @param {*} pin 
 * @returns 
 */
function CreateSliderContainer(pin) {
    var html = `
    <div x-pin="${pin}" class='slider-container'>
        <div class="slider-details">
            <span class="slider-details-pin"></span>
            <span class="slider-details-val ${css_normal}" title="Click for manual set.">0</span>
            <span>
                <select id="slider-mode-${pin}" name="slider-mode-${pin}" class="slider-mode" title="Set mode">
                    <option value="analogout">Analog Write</option>
                    <option value="digitalout">Digital Write</option>
                    <option value="servo">Servo</option>
                    <option value="tone">Tone</option>
                </select>
            </span>
            <span>
                <button class="slider-increment-btn slider-increment-minus">
                    <i class="bi bi-dash-circle"></i>
                </button>
            </span>
            <span>
                <button class="slider-increment-btn slider-increment-plus">
                    <i class="bi bi-plus-circle"></i>
                </button>
            </span>
            <span>
                <input type="checkbox" id="slider-enabled-${pin}" name="slider-enabled-${pin}" />
                <label for="slider-enabled-${pin}" >Enabled</label>
            </span>
        </div>
        <div class="slider-controls flex">
            <div class="">
                <button class="slider-controls-stop">
                    <i class="bi bi-sign-stop slider-controls-stop-icon"></i>
                </button>
            </div>
            <div class="flex-grow">
                <input type='range' step='${stepsize}' min='0' max='4095' value='0' class='slider-range' id='slider-${pin}'>
            </div>
            <div>
                <input class="form-control pulse-input pulse-input-value" type="number" id="pulse-value-${pin}" min="1" max="1000000"
                title="Pulse Value">
                <input class="form-control pulse-input pulse-input-time" type="number" id="pulse-time-${pin}" min="1" max="1000000"
                title="Pulse Time">
                <button type="button" class="btn btn-warning pulse-btn" id="pulse-btn-${pin}">Pulse</button>
            </div>
            <div class="">
                <button class="slider-controls-remove"><i class="bi bi-node-minus slider-controls-remove-icon"></i></button>
            </div>
        </div>
    </div>`;
    return $(html);
}
