#include "callbacks.h"

soundLibCallback logCallback;
soundLibCallback panicCallback;
soundStreamCallback inputStreamCallback;
soundStreamCallback outputStreamCallback;
floatPrintCallback audioStreamCallback;
charCallback audioStreamCallbackChar;
outputProcessedCallback outputProcessed;

void registerAudioPanicCallback(soundLibCallback func) {
    panicCallback = func;
}

void registerAudioLogCallback(soundLibCallback func) {
    logCallback = func;
}

void registerInputStreamCallback(soundStreamCallback func) {
    inputStreamCallback = func;
}

void registerOutputStreamCallback(soundStreamCallback func) {
    outputStreamCallback = func;
}

void registerFloatPrintCallback(floatPrintCallback func) {
    audioStreamCallback = func;
}

void registerCharCallback(charCallback func) {
    audioStreamCallbackChar = func;
}

void registerOutputProcessedCallback(outputProcessedCallback func) {
    outputProcessed = func;
}