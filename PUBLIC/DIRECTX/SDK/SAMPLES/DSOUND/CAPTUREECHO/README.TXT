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
      
      CaptureEcho [-d delay] [-t timeout] 

         -d approximate delay in milliseconds, default = 500
            NOTE: Delay must be greater than 500ms
         
         -t timeout in seconds, default = 30
            NOTE: Timeout must be greater than 4 times the delay.
                  It will also be converted to ms and quantized to the delay.


