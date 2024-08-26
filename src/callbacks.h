#ifndef CALLBACKS_H
#define CALLBACKS_H

/* print and error callback function helpers */
typedef void (*soundLibCallback) (char*);
typedef void (*soundStreamCallback) (char*, int);
typedef void (*floatPrintCallback) (char*, float);
typedef void (*charCallback) (char, int);
/* callback functions for audio playback */
typedef void (*outputProcessedCallback) (int);

void registerAudioPanicCallback(soundLibCallback func);

void registerAudioLogCallback(soundLibCallback func);

void registerInputStreamCallback(soundStreamCallback func);

void registerOutputStreamCallback(soundStreamCallback func);

void registerFloatPrintCallback(floatPrintCallback func);

void registerCharCallback(charCallback func);

void registerOutputProcessedCallback(outputProcessedCallback func);

extern soundLibCallback logCallback;
extern soundLibCallback panicCallback;
extern soundStreamCallback inputStreamCallback;
extern soundStreamCallback outputStreamCallback;
extern floatPrintCallback audioStreamCallback;
extern charCallback audioStreamCallbackChar;
extern outputProcessedCallback outputProcessed;

#endif