const MAX_PWM = 4095;
const MAX_TONE = 78000; // Manually tested with scope, esp32 stops at 78khz.
const MAX_SERVO = 180;
const MAX_COLOR = 255;

let BASE_URL = "";

const css_normal = "btn btn-light";
const css_busy = "btn btn-info";
const css_warning = "btn btn-warning";
const css_bad = "btn btn-danger";

var pinSettings = [
    { pin: 2, mode: 'analogout' }
];

var activity;
var terminal;

var mode;
var stepsize = 1;

var dataset = [];

const AnalogWrite = "analogout";
const AnalogRead = "analogin";
const DigitalWrite = "digitalout";
const DigitalRead = "digitalin";
const Servo = "servo";
const Tone = "tone";
const Integer = "integer";
const Color = "color";

const PinMode = { AnalogWrite, AnalogRead, DigitalWrite, DigitalRead, Servo, Tone, Integer, Color };

function addData(data) {
    var time = new Date();
    dataset.push(data.value);
    localStorage.setItem("dataset", dataset);
}

const GRID_X = 22;
const GRID_Y = 22;

function matrix_box(x, y) {
    return $(`<div id='matrix-box-${x}-${y}' x-row='${y}' x-col='${x}' class='matrix-box matrix-box-off'></div>`);
}

function matrix_row(y) {
    const row = $(`<div id='matrix-row-${y}' x-row='${y}' class='matrix-row'></div>`);

    for(var x = 0; x < GRID_X; x++) {
        row.append(matrix_box(x,y));
    }

    return row;
}

function matrix_set(x, y, on) {

    let r = 0;
    let g = 0;
    let b = 0;
    
    if (on) {
        r = $("#matrix-color-r").val();
        g = $("#matrix-color-g").val();
        b = $("#matrix-color-b").val();
    }

    PostMatrixCommand(x, y, r, g, b, function(response) {
        console.log("response", response);

        const box = $(`#matrix-box-${response.x}-${response.y}`);

        if (response.r == "0" && response.g == "0" && response.b == "0") {
            response.r = 0;
            response.g = 150;
            response.b = 0;
        }

        box.css("background-image", `radial-gradient(rgba(${response.r}, ${response.g}, ${response.b}, 0.75), black 120%)`);

    });

}

function matrix_box_onclick(evt) {
    console.log(evt);
    const y = evt.target.getAttribute('x-row');
    const x = evt.target.getAttribute('x-col');
    matrix_set(x, y, true);
}

function matrix_box_mouseover(evt) {
    //console.log(evt, evt.buttons);

    if (evt.shiftKey || evt.buttons == 1) {
        const y = evt.target.getAttribute('x-row');
        const x = evt.target.getAttribute('x-col');
        matrix_set(x, y, true);
        return;
    }

    if (evt.ctrlKey|| evt.buttons == 2) {
        const y = evt.target.getAttribute('x-row');
        const x = evt.target.getAttribute('x-col');
        matrix_set(x, y, false);
        return;
    }
}

function build_matrix(root_element) {

    for(var y = 0; y < GRID_Y; y++) {
        root_element.append(matrix_row(y));
    }

    document.querySelectorAll(".matrix-box").forEach(element => {
        element.addEventListener("click", matrix_box_onclick); 
    });
    document.querySelectorAll(".matrix-box").forEach(element => {
        element.addEventListener("mouseover", matrix_box_mouseover); 
    });
    document.querySelectorAll(".matrix-box").forEach(element => {
        element.addEventListener("mouseover", matrix_box_mouseover); 
    });


}

$(function(){
    document.addEventListener('contextmenu', (event) => {
        event.preventDefault();
        const y = evt.target.getAttribute('x-row');
        const x = evt.target.getAttribute('x-col');
        matrix_set(x, y, false);
    });

    var matrix = document.getElementById("matrix");
    if (matrix) {
        matrix = $(matrix);

        build_matrix(matrix);

        document
            .getElementById("matrix-clear")
            .addEventListener("click", (evt) => {
                PostApiCommand("led clear");
                matrix.empty();
                build_matrix(matrix);

            });

        document
            .getElementById("matrix-color-a")
            .addEventListener("change", (evt) => {
                const a = $("#matrix-color-a").val();
                PostApiCommand(`led brightness ${a}`);
            });

    }

});

$(function () {

    const urlInput = document.getElementById("hosturl-input");
    if (urlInput) {
        urlInput.addEventListener("change", (evt) => {
                console.log(evt.target.value);
                BASE_URL = evt.target.value;
                localStorage.setItem("hosturl-input", BASE_URL);
            });
        const stored_url = localStorage.getItem("hosturl-input");
        
        BASE_URL = stored_url || "";

        urlInput.value = BASE_URL;
    }

    terminal = document.getElementById("terminal-history");

    if (terminal) {
        terminal = $(terminal);
        document
            .getElementById("terminal-input")
            .addEventListener("keydown", handleTerminalKeypress);
    }

    activity = document.getElementById("activity");

    if (activity) {
        activity = $(activity);

        const sweepModal = document.getElementById('sweepModal');
        if (sweepModal) {
            sweepModal.addEventListener('show.bs.modal', event => {
                console.log(event);
                var sweepSettings = localStorage.getItem("sweepSettings")
                if (sweepSettings) {
                    updateSweepModal(JSON.parse(sweepSettings));
                }
            });

            $(".sweep-submit-btn").on("click", handleSweepSubmit);
        }

        var cached_pins = localStorage.getItem("pinSettings");
        if (cached_pins != null && cached_pins !== "") {
            pinSettings = JSON.parse(cached_pins);
        }

        stepsize = localStorage.getItem("stepsize") || "1";
        $("#stepsize").val(stepsize).change();

        document
            .getElementById("command-input")
            .addEventListener("keydown", handleTerminalKeypress);

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

    }
});

const command_history = [];
let history_selected = 0;


function terminalSubmit(target) {
    const command = target.value;
    command_history.unshift(command);
    history_selected = 0;

    const commands = command.split("\n");

    commands.forEach(cmd => {
        if (cmd != "") {
            PostApiCommand(cmd, function(response) {
                target.value = "";
            });
        }
    });
}

function handleTerminalKeypress(evt) {
    var use_ctrl = evt.target.type === "textarea";

    if (use_ctrl) {
        if (evt.ctrlKey && evt.keyCode == 38) {
            const hist = command_history[history_selected];
            evt.target.value = hist;
            history_selected++;
            if (history_selected > command_history.length) {
                history_selected = 0;
            }
        }

        if (evt.ctrlKey && evt.keyCode == 13) {
            terminalSubmit(evt.target);
        }
    } else {
        if (evt.keyCode == 38) {
            const hist = command_history[history_selected];
            evt.target.value = hist;
            history_selected++;
            if (history_selected > command_history.length) {
                history_selected = 0;
            }
        }

        if (evt.keyCode == 13) {
            terminalSubmit(evt.target);
        }
    }
}


function handleCommandKeypress(evt) {
    if (evt.keyCode == 38) {
        const hist = command_history[history_selected];
        
        $("#command-input").val(hist);
        history_selected++;
        if (history_selected > command_history.length) {
            history_selected = 0;
        }
    }

    if (evt.keyCode == 13) {
        const command = $("#command-input").val();
        command_history.unshift(command);
        history_selected = 0;
        PostApiCommand(command, function(response) {
            $("#command-input").val("");
        });
    }
}

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

    if(!confirm("Start Sweep?")) {
        return;
    }

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

function Command_Click(evt) {
    var command = prompt("Enter Command", ``);
    if (!command || command == null || command == "") {
        return;
    }
    PostApiCommand(command, function(response) {
        console.log('response', response);
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

    if (isNaN(pin) || alreadySet || pin < 2 || pin > 64) {
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
    AddActivity(message, `<span class="act-in"><i class="bi bi-telephone-inbound act-icon"></i></span>`);
}

function OutActivity(message) {
    AddActivity(message, `<span class="act-out"><i class="bi bi-telephone-outbound act-icon"></i></span>`);
}

function errActivity(message) {
    AddActivity(message, `<span class="act-bug"><i class="bi bi-bug act-icon"></i></span>`);
}

function AddActivity(message, direction) {
    var d = new Date();
    if (typeof message === "object") {
        message = JSON.stringify(message);
    }
    var hr = d.getHours();
    if(hr > 12) {
        hr = hr - 12;
    }

    var time = `${String(hr).padStart(2, '0')}:${String(d.getMinutes()).padStart(2, '0')}:${String(d.getSeconds()).padStart(2, '0')}.${String(d.getMilliseconds()).padStart(3, '0')}`;

    var item = `<tr>
        <td>${time}</td>
        <td>${direction}</td>
        <td>${message}</td>
    </tr>`;

    if (activity) {
        activity.prepend($(item));
    }

    if (terminal) {
        terminal.prepend($(item));
    }
    
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

function PostMatrixCommand(x, y, r, g, b, onComplete) {
    var url = BASE_URL + "/mat";

    var data = { x, y, r, g, b };

    OutActivity({ url, ...data }, true);

    $.post(url, data, function (response) {
        InActivity(response);
        if (typeof(onComplete) == 'function') {
            onComplete(response);
        }
    }).fail(function (jqxhr, textStatus, errorThrown) {
        errActivity(jqxhr.statusCode());
    }).always(function() {});
}


function PostApiCommand(command, onComplete) {
    var url = BASE_URL + "/api";

    var data = { cmd: command };

    OutActivity({ url, ...data }, true);

    $.post(url, data, function (response) {
        InActivity(response);
        if (typeof(onComplete) == 'function') {
            onComplete(response);
        }
    }).fail(function (jqxhr, textStatus, errorThrown) {
        errActivity(jqxhr.statusCode());
    }).always(function() {});
}

function PostSweep(data) {
    updatePending = true;
    var url = BASE_URL + "/sweep";

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
    PostApiCommand("reset", function() {
        RebuildPins();
    });
}

var updatePending = false;
function PostPinValue(mode, pin, value, valueSpan) {
    updatePending = true;

    var url = BASE_URL + `/${mode}`;
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

    var url = BASE_URL + `/pulse`;
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
    var url = BASE_URL + `/analogin`;
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
    var sliderEnabled = elem.find("input.slider-enabled");
    var settingsBtn = elem.find("button.slider-settings");

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
            case "color":
                max = MAX_COLOR;
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
            case "color":
                max = MAX_COLOR;
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
        sliderEnabled.prop("checked", false).trigger("change");
        slider.val(Number(slider.val()) + 1).trigger("input").change();
    });

    var decrementBtn = elem.find("button.slider-increment-minus");
    decrementBtn.on("click", function (evt) {
        sliderEnabled.prop("checked", false).trigger("change");
        slider.val(Number(slider.val()) - 1).trigger("input").change();
    });

    sliderEnabled.on("change", function (evt) {
        var checked = sliderEnabled.prop("checked");
        slider.prop("disabled", !checked);
    });

    settingsBtn.on("click", function (evt) {
        let modal = $('#configModal');
        modal.modal('show');
        modal.find(".config-header").html(`Pin ${pin} Config`);
        var config = _.find(pinSettings, (i) => {
            return i.pin == pin;
        });
        console.log("config", config);
        modal.find("input.config-label").val(`PIN ${pin}`);


    });

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
            <span class="slider-details-val ${css_normal}">0</span>
            <span class="slider-details-mode" >
                <select id="slider-mode-${pin}" name="slider-mode-${pin}" class="slider-mode" title="Set mode">
                    <option value="analogout">Analog Write</option>
                    <option value="digitalout">Digital Write</option>
                    <option value="servo">Servo</option>
                    <option value="tone">Tone</option>
                    <option value="integer">Integer</option>
                    <option value="color">Color</option>
                </select>
            </span>
            <span>
                <button class="slider-increment-btn slider-increment-minus">
                    <i class="bi bi-dash-circle"></i>
                </button>
                <button class="slider-increment-btn slider-increment-plus">
                    <i class="bi bi-plus-circle"></i>
                </button>
            </span>
            <div class="form-check form-switch">
                <input class="form-check-input slider-enabled" type="checkbox" value="" role="switch" id="slider-enabled-${pin}" checked>
                <label class="form-check-label" for="slider-enabled-${pin}">
                    Enabled
                </label>
            </div>
            <span>
                <button id="slider-settings-${pin}" class="btn btn-outline-primary slider-settings" type="button">
                    <i class="bi bi-gear"></i>
                </button>
            </span>
            <div class="">
                <button class="slider-controls-remove slider-settings">
                    <i class="bi bi-node-minus slider-controls-remove-icon"></i>
                </button>
            </div>
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
        </div>
    </div>`;
    return $(html);
}
