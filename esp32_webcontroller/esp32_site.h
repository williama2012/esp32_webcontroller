#ifndef ESP32_SITE_H
#define ESP32_SITE_H
#include <Arduino.h>
#include "incbin.h"
INCTXT(WebPage, "index.min.html");
INCTXT(MatrixWebPage, "matrix.min.html");
INCTXT(WebJavascript, "index.min.js");
INCTXT(WebStylesheet, "index.min.css");

#endif