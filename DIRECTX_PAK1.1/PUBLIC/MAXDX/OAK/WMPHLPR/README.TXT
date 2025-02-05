//-----------------------------------------------------------------------------
// 
// Sample Name: Windows Media Player Helper Sample
// 
// Copyright (c) 2000 Microsoft Corporation. All rights reserved.
// 
//-----------------------------------------------------------------------------


Description
===========
  This dll demonstrates how to integrate the Windows Media Player control with the
  browser in a single window UI configuration.

User's Guide
============
  Many CE based devices have custom user interface configuration.  Because of this
  the Windows Media Player is shipped as an ActiveX control only. This allows an 
  OEM to provide their own application that hosts the Windows Media Player
  control.

  Other CE based devices don't use an overlapped windows type of user interface.
  It is for this type of application that the sample wmphlpr.dll is used -
  to show how Windows Media Playback capability can be added to the web browser
  without having a separate application.

  The wmphlpr.dll registers itself (look at public\maxdx\oak\files\project.reg)
  as the ActiveX object that supports all of the media types supported by the
  Windows Media Player control in the maxdx configuration. For any browser
  navigation to a Windows Media Player media type, i.e:
    <a href="http://www.myserver.com/mymedia.asx">my media</a>
  the browser will load wmphlpr.dll and call the IPersist::Load method with a
  moniker to the media URL. This simple sample packages up that URL into a
  simple structure and posts a message to the instance of IESample that the
  navigation is taking place. The IESample program then navigates to a simple
  html page (media.htm) that receives the target media URL and initializes the
  Windows Media Player control with that URL with simple jscript code.
    
Programming Notes
=================
  wmphlpr.dll and IESample.exe are matched to work together.
