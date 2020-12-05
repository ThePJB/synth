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


--------------
Fixed a sequencer bug. Good to make a test file so i can have prints to debug lol
it was >= instead of >. classic (off by) 1
--------------

so added ring buffer. up next delay lines and fir filters.

to do also ring mod and convolution

do reverb, echo or convolution?
    echo is a single instance (easy), do that first, just samples n and volume v
    reverb is the superposition of many echoes, so like feedback
    convolution is probably a way of doing reverb


also probably a refactor is due

--------------

so yeah theres a bug with the new way of doing things where sound is weird. lets try drawing something and maybe use that to debug, e.g. draw connections and show if they are alive

---------------

nb crashing in calloc call 51 alloc block.
is it 1st one?

must be smashing the heap before then (corrupted top size)