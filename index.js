const MAX_PWM = 4095;
const MAX_TONE = 40000; //4294967295;
const MAX_SERVO = 360;

const css_normal = "btn btn-light";
const css_busy = "btn btn-info";
const css_warning = "btn btn-warning";
const css_bad = "btn btn-danger";

var pinSettings = [
    { pin: 2, mode: 'analogout' }
];

var activity;
var mode;
var stepsize = 1;

var dataset = [];

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

    const sweepModal = document.getElementById('sweepModal');
    if (sweepModal) {
        sweepModal.addEventListener('show.bs.modal', event => {
            console.log(event);
            var sweepSettings = localStorage.getItem("sweepSettings")
            if (sweepSettings) {
                updateSweepModal(JSON.parse(sweepSettings));
            }
        });

        $("#sweep-submit-btn").on("click", handleSweepSubmit);
    }

    var cached_pins = localStorage.getItem("pinSettings");
    if (cached_pins != null && cached_pins !== "") {
        pinSettings = JSON.parse(cached_pins);
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
        .addEventListener("click", PostHardResetAllPins);

    RebuildPins();
});

function handleSweepSubmit(evt) {
    var settings = {
        servo: $("#sweep-servo").val(),
        pwm: $("#sweep-pwm").val(),
        value: $("#sweep-value").val(),
        low: $("#sweep-low").val(),
        high: $("#sweep-high").val(),
        count: $("#sweep-count").val(),
        delay: $("#sweep-delay").val(),
    };

    localStorage.setItem("sweepSettings", JSON.stringify(settings));

    console.log('save sweep:', settings.servo, settings.pwm, settings.value, settings.low, settings.high, settings.count, settings.delay);

    PostSweep(settings);
}

function updateSweepModal(settings) {
    console.log("updateSweepModal", settings);
    $("#sweep-servo").val(settings.servo);
    $("#sweep-pwm").val(settings.pwm);
    $("#sweep-value").val(settings.value);
    $("#sweep-low").val(settings.low);
    $("#sweep-high").val(settings.high);
    $("#sweep-count").val(settings.count);
    $("#sweep-delay").val(settings.delay);
}


function SavePins() {
    var save = JSON.stringify(pinSettings);

    console.log("saving", save);

    localStorage.setItem("pinSettings", save);
}

function RebuildPins() {
    $(".slider-set").empty();

    pinSettings.forEach((value) => {
        AddSliderContainer(value.pin, value)
    });
}

function AddNewSlider_Click(evt) {
    var max = 2;

    var m = _.max(pinSettings, (i) => { return i.pin; }).pin;

    if (m) {
        max = m + 1;
    }

    var pin = 0;

    if (evt.ctrlKey) {
        pin = max;
    } else {
        pin = prompt("Enter Pin Number", `${max}`);
        if (!pin || pin == null || pin == "") {
            return;
        }
        pin = Number(pin);
    }

    var alreadySet = (_.find(pinSettings, (i) => {
        return i.pin == pin;
    })) != null;

    if (isNaN(pin) || alreadySet || pin < 2 || pin > 24) {
        return;
    }

    var settings = { pin: pin, mode: 'analogout' };

    pinSettings.push(settings);

    AddSliderContainer(settings.pin, settings);
    SavePins();
}

function AddSliderContainer(pin, settings) {
    var slideContainer = CreateSliderContainer(pin);
    BindContainer(pin, slideContainer, settings);
    $(".slider-set").append(slideContainer);
}

/**
 * @description Bind UI fields for pin group
 * @param {*} i 
 * @param {*} obj 
 */
function BindContainer(i, obj, settings) {
    var elem = $(obj);
    var pin = elem.attr("x-pin");

    elem.find("span.slider-details-pin").html(`PIN ${pin}`);

    var mode = elem.find("select.slider-mode");
    var slider = elem.find("input.slider-range");
    var valueSpan = elem.find("span.slider-details-val");

    //slider-enabled

    var max = MAX_PWM;
    if (settings) {
        mode.val(settings.mode || "analogout");

        switch (mode.val()) {
            case "tone":
                max = MAX_TONE;
                break;
            case "servo":
                max = MAX_SERVO;
                break;
            case "digitalout":
                max = 1;
                break;
        }
        slider.attr("max", max);
    }

    mode.on("change", function () {
        var max = MAX_PWM;
        switch (this.value) {
            case "tone":
                max = MAX_TONE;
                break;
            case "servo":
                max = MAX_SERVO;
                break;
            case "digitalout":
                max = 1;
                break;
        }

        slider.attr("max", max);
        var save = _.find(pinSettings, (p) => {
            return p.pin == pin;
        });
        if (save) {
            save.mode = this.value;
        }

        SavePins();
    });

    slider.on("change", function (evt) {
        $(valueSpan)
            .removeClass(css_normal)
            .addClass(css_warning)
            .removeClass(css_busy)
            .removeClass(css_bad);

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

        var idx = _.findIndex(pinSettings, (i) => {
            return i.pin == pin;
        })

        pinSettings.splice(idx, 1);
        RebuildPins();
        localStorage.setItem("pinSettings", JSON.stringify(pinSettings));
    });


    var pulseValue = elem.find("input.pulse-input-value");
    var pulseTime = elem.find("input.pulse-input-time");
    var pulseBtn = elem.find("button.pulse-btn");

    pulseBtn.on("click", function (evt) {
        PostPulsePin(pin, pulseValue.val(), pulseTime.val(), valueSpan);
    });

    var incrementBtn = elem.find("button.slider-increment-plus");
    incrementBtn.on("click", function (evt) {
        slider.val(Number(slider.val()) + 1).trigger("input").change();
    });

    var decrementBtn = elem.find("button.slider-increment-minus");
    decrementBtn.on("click", function (evt) {
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

        refreshInterval = setInterval(RefreshPinValues, time);

        return;
    }

    clearInterval(refreshInterval);

    RefreshPinValues();
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

    $("input.slider-range").attr("step", this.value);
}

function activity_click(evt) {
    if (evt.ctrlKey) {
        $("#activity").empty();
    }
}

//#region === API ===

function PostSweep(data) {
    updatePending = true;
    var url = "/sweep";

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


function PostHardResetAllPins(evt) {
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
    var pins = _.map(pinSettings, (i) => {
        return i;
    })

    var data = { pins: JSON.stringify(pins) };

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
        <div class="slider-details flex">
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
            <div class="form-check">
                <input class="form-check-input slider-enabled" type="checkbox" value="" id="slider-enabled-${pin}" checked>
                <label class="form-check-label" for="slider-enabled-${pin}">
                    Enabled
                </label>
            </div>
            <span>
                <button id="settingsBtn" class="btn btn-outline-primary" type="button" data-bs-toggle="modal" data-bs-target="#configModal">
                <i class="bi bi-gear"></i>
                </button>
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
