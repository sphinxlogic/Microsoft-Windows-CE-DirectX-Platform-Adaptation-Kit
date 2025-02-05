------------------------------------------
Running the CaptureEcho sample application
------------------------------------------

The Direct Sound CaptureEcho application will continuously capture audio
information from a Direct Sound capture buffer and play it back using
a streaming Direct Sound buffer.  The total execution time and the
size of the transfer block are configurable with command line options.

Follow these steps to capture and playback audio data with CaptureEcho:

1) Initialize a CESH session

2) To begin capture using the default settings

    s CaptureEcho


    Usage:

      CaptureEcho [-d delay] [-t timeout] [-f format]

         -d approximate delay in milliseconds, default = 500
            NOTE: Delay must be at least 500ms

         -t timeout in seconds, default = 10
            NOTE: Timeout must be greater than 4 times the delay.
                  It will also be converted to ms and quantized to the delay.

         -f requested capture buffer format (0-11)


The capture buffer format is selectable from the following table:

    0   11kHz 8-bit  mono
    1   11kHz 8-bit  stereo
    2   11kHz 16-bit mono
    3   11kHz 16-bit stereo
    4   22kHz 8-bit  mono
    5   22kHz 8-bit  stereo
    6   22kHz 16-bit mono
    7   22kHz 16-bit stereo
    8   44kHz 8-bit  mono
    9   44kHz 8-bit  stereo
    10  44kHz 16-bit mono
    11  44kHz 16-bit stereo