DirectShow Sample -- playall.exe
---------------------------------

Usage:

      PlayAll [-p search path] [-m mediafile]
              [-l #loops] [-g #global loops] [-c] [-r] [-e]
              [-k #skips] [-i index] [-x maxsize] [-n minsize] [-t seconds]


      -p specifies a path from which to begin playing all supported media

      -m specifies a media file to play (or multiple files if a wildcard
             argument is used)

      Note: Path (-p) and media filename (-m) options are mutually exclusive!

      -l specifies number of times to loop each media file
      -g specifies number of times to loop ENTIRE selection
      -c centers video at default size instead of fullscreen

      -r starts a recursive search, allowing you to play all relevant files
             on a server (use with -p or with -m wildcards)

      -e displays DirectShow events as they are received
      -k specifies number of files to skip at startup (for wildcards)
      -i specifies index into media table for -p startup (see mediatypes.h)

      -x specifies the max file size to play (upper bound)
      -n specifies the min file size to play (lower bound)
      -t specifies the max number of seconds to play each file

      WILDCARDS are supported for media file parameter.


Description:

    This sample is an automated audio/video media file player.
    It uses DirectShow to play any supported audio or video media file
    (MPG, AVI, QT, WAV, AU, SND, MID, etc.).  Video files appear by default
    as a full-screen video using the IVideoWindow DirectShow interface.


    Mutually exclusive options:
    ---------------------------

    Search path:  -p path
        Set this to the directory containing your media files if you want
        PlayAll to run through them automatically.

        If you use the "-p path" option, PlayAll will play all valid media in
        a directory (and optionally, recursively with the "-r" option).
        Valid media is defined by the product and is conditionally compiled
        based on your system's capabilities.

    Media file(s):  -m file or wildcard
        Specify a single file (like "\windows\chicken1.mpg") or multiple files
        using a wildcard name (like "\windows\*.mpg" or "\\myserver\*.mpg").


    Other options:
    --------------

    Per-file looping:  -l loops
        Number of times to loop each selected file before moving on

    Global looping of the entire set of selected media:  -g loops
        Number of times to cycle through the ENTIRE media selection,
        whether it be a wildcard filename or the valid media in
        a directory specified with the "-p" option.

    Center at normal size (instead of the fullscreen default):  -c
        By default, video is full-screen.  "-c" will center it at normal size.

    Recursively search for valid media:  -r
        When the -r option is specified, PlayAll will search for and play ALL
        valid media recursively, starting at the directory specified with the
        -p path option OR starting with a wildcard filename (when using "-m").

        For example, setting the path to "\\myserver\mediafiles" and using "-r"
        would search for and play all supported media on the server's directory
        tree beginning at the "mediafiles" directory.


Accessing media files

    The media file may exist in the Windows CE image or on a network server.

  If the file is on a network server:
    You may read a media file from a network server if you provide the full network path.
    For example, "s playall -p \\myserver\mediafiles\video\*.avi" will play
    all AVI files in the "mediafiles\video" directory on server "myserver".


Adding a media file to the image:

    To put the file in the image, add the following lines to the end of
    PROJECT.BIB in your release directory and rebuild the image with MAKEIMG.BAT:

    FILES
        <filename>    $(_FLATRELEASEDIR)\<filename>    NK  SHU

    If a FILES section already exists, add only the second line to the end
    of the existing FILES section.


Limitations:

    This sample will only render media files that are supported by the
    DirectShow subsystem.  If you attempt to play video (AVI, QuickTime, MPEG)
    that is encoded with an unsupported Codec, you will only see a black
    screen or no visible change to the display, although you should hear the
    associated audio component if it uses a supported format.

