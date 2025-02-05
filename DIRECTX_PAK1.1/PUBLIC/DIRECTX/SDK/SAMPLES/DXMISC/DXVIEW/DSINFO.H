/****************************************************************************

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.

    Copyright (C) 1995-2000 Microsoft Corporation. All Rights Reserved.

    PROGRAM: dsinfo.h

    PURPOSE: DirectX Device Viewer DirectSound-specific header file

****************************************************************************/


#define DSCAPDEF(name,val,flag) {name, FIELD_OFFSET(DSCAPS,val), flag}
#define DSVALDEF(name,val)      {name, FIELD_OFFSET(DSCAPS,val), 0}

#define DSCCAPDEF(name,val,flag) {name, FIELD_OFFSET(DSCCAPS,val), flag}
#define DSCVALDEF(name,val)      {name, FIELD_OFFSET(DSCCAPS,val), 0}

extern IDirectSound *lpDS;                 // DirectSound object
extern GUID * dsid;

extern IDirectSoundCapture *lpDSC;         // DirectSoundCapture object
extern GUID * dscid;
