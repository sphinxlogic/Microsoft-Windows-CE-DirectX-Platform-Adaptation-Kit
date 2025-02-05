-------------------------------------
Running the DSPlay application
-------------------------------------

The Direct Sound DSPlay application will play a standard wave file (.wav)
over a CESH link using the Direct Sound API.  You may also provide a
loop count to cause the sound to be looped any number of times (up to 32767).
To run the tests, start a CESH session and open a serial output window.

Follow these steps to play a file with DSPlay:

1) Initialize a CESH session

2) Copy any wave files that you want to play into your RELEASE directory.
   (or the same directory from where you run CESH and store your BIN files.

3) Play the file with the following command:

      s dsplay <wavefile> <optional loop count>

   File information will be displayed before the wave file plays.
   If no loop count is specified, the default value '1' will be used.

   Example:   s dsplay tada.wav
              s dsplay homer22.wav 10

