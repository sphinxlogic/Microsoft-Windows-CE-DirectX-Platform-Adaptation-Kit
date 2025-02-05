DirectShow Sample -- playwnd.exe
---------------------------------

Usage:

    playwnd <media filename>

Description:

    This sample is an interactive audio/video media file player.
    It uses DirectShow to play any supported audio or video media file
    (MPG, AVI, QT, WAV, AU, SND, MID, etc.).  The video will appear in
    a window on the screen, and you can use a mouse to move the window.

    If the media has a video component, PlayWnd will read the video's
    default size and adjust the player's client area to allow the video
    to play at its exact default size (taking into account the size of
    caption bar & borders).  When the media file is playing, the menu bar
    will disappear, giving you keyboard control with 'P' and 'S' for
    pause/play and stop/reset.  

    You may mute the audio by pressing 'M'.  You may toggle full-screen mode
    by pressing 'F'.  To return to the menu, hit ESC, F12, X, or Q.

    If the media is audio-only, the player will display a small window area -
    white while an audio file is loaded or playing, or grey when it is closed
    (during selection of another file, for example).  The main menu and its
    control submenu are always visible while playing audio files.

    You can specify a media file as the only command line argument:
    	Ex:  s playwnd \\myserver\mediafiles\video\sample.avi

    If no file is specified, the application will automatically display the
    Windows OpenFile dialog so that you can choose a file.

    Use the menu bar or the close button to exit the application.


Accessing media files

    The media file may exist in the Windows CE image or on a network server.

 If the file is on a network server:
    You may read a media file from a network server if you provide the full URL.
    For example, "s playwnd http://myserver/mediafiles/video/101.avi" will
    read the file from the server into system memory.  This approach will fail
    if the media file is too large to fit in memory.


Adding a media file to the image:

    To put the file in the image, add the following lines to the end of
    PROJECT.BIB in your release directory and rebuild the image with MAKEIMG.BAT:

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
    to the Windows CE device or through the application's main menu bar.

    Keyboard                    Action
    --------                    ------
    P                           Play/Pause toggle
    S                           Stop and Rewind to beginning
    M                           Audio mute toggle
    F                           Full-screen mode toggle
    Quit to menu                ESC or Q or X or F12
