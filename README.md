todos:

open stream and all that
purge pulseaudio

sequencer

delay lines and filters

visual block placement and stuff

for (int i = 0, end = Pa_GetDeviceCount(); i != end; ++i) {
    PaDeviceInfo const* info = Pa_GetDeviceInfo(i);
    if (!info) continue;
    printf("%d: %s\n", i, info->name);
}