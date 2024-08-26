#ifndef CALLBACKS_H
#define CALLBACKS_H

#include "csoundlib.h"

extern soundLibCallback logCallback;
extern soundLibCallback panicCallback;
extern soundStreamCallback inputStreamCallback;
extern soundStreamCallback outputStreamCallback;
extern floatPrintCallback audioStreamCallback;
extern charCallback audioStreamCallbackChar;
extern outputProcessedCallback outputProcessed;

#endif