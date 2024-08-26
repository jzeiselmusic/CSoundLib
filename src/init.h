#ifndef AUDIO_INIT_H
#define AUDIO_INIT_H

int lib_startSession(int sample_rate, int bit_depth);

int lib_destroySession();

int lib_getCurrentBackend();

int _checkEnvironmentAndBackendConnected();

#endif 