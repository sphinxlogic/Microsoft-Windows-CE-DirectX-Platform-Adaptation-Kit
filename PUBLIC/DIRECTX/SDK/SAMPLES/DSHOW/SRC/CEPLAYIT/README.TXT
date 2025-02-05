DirectShow Sample -- ceplayit.exe
---------------------------------

Usage:

    ceplayit <media filename> <options>

Description:

    This sample is an interactive audio/video media file player.
    It uses DirectShow to play any supported audio or video media file
    (MPEG, AVI, QT, WAV, AU, SND, etc.).  The video window will float
    freely on the screen, and you can control its size, position, and
    playback state with an attached keyboard.

    Press ESC to exit the application.


If your Windows CE configuration includes a shell component:

    If the media file is an audio-only file, you will see one entry in
    the taskbar (CEPlayit: <filename>).  If your media file contains
    a video component, a second window will appear with the title
    "ActiveMovie Window".  Video will be rendered within the ActiveMovie
    window, but the application is controlled via the CEPlayit window.

    To control the application (to pause/play video, move or resize the
    video screen, exit, etc.), the "CEPlayit" window must have focus.


Accessing media files

    The media file may exist in the Windows CE image, on a network server,
    or in your attached PC's WinCE release directory (_FLATRELEASEDIR).

 If the file is in the image:
    If a full path is not specified, the default path for the media file
    is \Windows. If a media file doesn't exist in that directory,
    the application looks in the root directory (\) before it displays
    the error message "Media file not found."

 If the file is on a network server:
    You may read a media file from a network server if you provide the full URL.
    For example, "s ceplayit http://myserver/mediafiles/video/101.avi" will
    read the file from the server into system memory.  This approach will fail
    if the media file is too large to fit in memory.

    When streaming is supported, you will also be able to use MMS: prefixes to
    stream the file.  For example, "s ceplayit mms://myserver/<myfile.xyz>"
    would stream <myfile.xyz> from <myserver>.  Streaming requires Windows
    Media Technologes components (formerly known as NetShow).

 If the file is on an attached PC:
    You may read a media file through the CESH connection by prefixing the
    filename with "cesh:" (no quotes).  For example, "s ceplayit cesh:101.avi"
    will read 101.avi from the attached PC's release directory (specified by
    the global environment variable _FLATRELEASEDIR).

    NOTE: When playing a media file through CESH, the following steps occur:
       -  Create a temporary file on the CE device (\windows\<filename>),
       -  Copy the media through the CESH link to the temporary file,
       -  Play the media, and
       -  Delete the temporary file

    If there is insufficient free storage space on the Windows CE device, the
    file copy will fail and the program will display some useful memory
    information before exiting.


Adding a media file to the image:

    To put the file in the image, add the following lines to the end of
    PROJECT.BIB in your release directory and rebuild the image with MAKEIMG.EXE:

    FILES
        <filename>    $(_FLATRELEASEDIR)\<filename>    NK  SHU

    If a FILES section already exists, add only the second line to the end
    of the existing FILES section.


Limitations:

    This sample will only render media files that are supported by the
    DirectShow subsystem.  If you attempt to play a video (AVI, QuickTime, MPEG)
    that is encoded with an unsupported Codec, you will only see a black
    screen or no visible change to the display, although you should hear the
    associated audio component if it uses a supported format.


User Input:

    Simple user input is supported through a keyboard connected
    to the Windows CE device.

    Keyboard                    Action
    --------                    ------
    P                           Play/Pause toggle
    S                           Stop and Rewind
    R                           Reset to defaults and play from beginning

    F                           Toggle full-screen video
    L                           Toggle looping (on/off)
    C                           Center video

    Up/Down/Left/Right arrow    Move video window
    SHIFT + arrow keys          Stretch / shrink video window
    + or -                      Increase/decrease video size and center

    F1-F4                       Position video in one screen quadrant
    F5-F8                       Position video in two screen quadrants
    F9                          Position video in default web client area
    F10                         Reset to default size and center video

    Quit                        ESC or Q or X or F12


Command Line Parameters:


Exclusive options (not valid with any other command line option):
  -z       List current running player instances
  -z<file> Set keyboard focus to instance playing <file>

Command line options (any combination):
  -l    Enable looping of video file
  -f    Play fullscreen
  -k    Keep copied CESH file in image on exit (don't delete temporary file)
  -c    Center video
  -x<#> Set X coordinate
  -y<#> Set Y coordinate
  -w<#> Set video width
  -h<#> Set video height
  -q<#> Initialize video to given quadrant (1-10)


Usage examples:

CEPlayit <media file> <options>
  Ex: CEPlayit small.avi                 (Play file in CE image)
  Ex: CEPlayit cesh:small.avi            (Play file through CESH link)
  Ex: CEPlayit http://myserver/small.avi (Play file on HTTP server)
  Ex: CEPlayit cesh:small.avi -l -x100 -y100 -w160 -h120
  Ex: CEPlayit -z                        (List all running instances)
  Ex: CEPlayit -zgo.avi                 (Set focus to instance playing go.avi)
  Ex: CEPlayit cesh:small.avi -k -l -q4  (Play small.avi in quadrant 4, loop,
                                           keep temporary file on CE device)

