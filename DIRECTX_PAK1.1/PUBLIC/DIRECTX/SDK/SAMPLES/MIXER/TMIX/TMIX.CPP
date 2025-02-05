//---------------------------------------------------------------------------
// tmix.cpp - a little test program for playing with mixer api
//
// Probes the mixer devices in the system to enumerate
// their caps, lines and controls.
//
// Note: Also deliberately calls mixer functions with bad args
// to verify robustness of mixer subsystem.
//
// Copyright (c) 1999-2000 Microsoft Corporation. All rights reserved.
//---------------------------------------------------------------------------


#include <windows.h>
#include <stdio.h>
//#include <mmreg.h> // multimedia vendor/product/format code definitions

#define NELEMS(a) (sizeof(a)/sizeof((a)[0]))

#ifdef UNDER_CE
#define SSTR "%ls"
#else
#define SSTR "%s"
#endif

typedef struct
{
	int	code;
	const char * name;
	const char * msg;
} MMSYSCODE;

#define MMCODE(code,msg) {code,#code,msg}

const char * get_mmcode(MMSYSCODE table[], int nelems, int code);
const char * get_mmmsg(MMSYSCODE table[], int nelems, int code);

#define MMNAME(tab,cod) get_mmcode(tab,NELEMS(tab),cod)
#define MMDESC(tab,cod) get_mmmsg(tab,NELEMS(tab),cod)

MMSYSCODE mmcode_table[] =
{
 MMCODE(MMSYSERR_NOERROR	  ,"no error")
,MMCODE(MMSYSERR_ERROR        ,"unspecified error")
,MMCODE(MMSYSERR_BADDEVICEID  ,"device ID out of range")
,MMCODE(MMSYSERR_NOTENABLED   ,"driver failed enable")
,MMCODE(MMSYSERR_ALLOCATED    ,"device already allocated")
,MMCODE(MMSYSERR_INVALHANDLE  ,"device handle is invalid")
,MMCODE(MMSYSERR_NODRIVER     ,"no device driver present")
,MMCODE(MMSYSERR_NOMEM        ,"memory allocation error")
,MMCODE(MMSYSERR_NOTSUPPORTED ,"function isn't supported")
,MMCODE(MMSYSERR_BADERRNUM    ,"error value out of range")
,MMCODE(MMSYSERR_INVALFLAG    ,"invalid flag passed")
,MMCODE(MMSYSERR_INVALPARAM   ,"invalid parameter passed")
,MMCODE(MMSYSERR_HANDLEBUSY   ,"handle being used simultaneously on another thread (eg callback)")
,MMCODE(MMSYSERR_INVALIDALIAS ,"specified alias not found")
,MMCODE(MMSYSERR_BADDB        ,"bad registry database")
,MMCODE(MMSYSERR_KEYNOTFOUND  ,"registry key not found")
,MMCODE(MMSYSERR_READERROR    ,"registry read error")
,MMCODE(MMSYSERR_WRITEERROR   ,"registry write error")
,MMCODE(MMSYSERR_DELETEERROR  ,"registry delete error")
,MMCODE(MMSYSERR_VALNOTFOUND  ,"registry value not found")
,MMCODE(MMSYSERR_NODRIVERCB   ,"driver does not call DriverCallback")
,MMCODE(MMSYSERR_LASTERROR    ,"last error in range")
};

#define MMERRNAME(cod) MMNAME(mmcode_table,cod)
#define MMERRDESC(cod) MMDESC(mmcode_table,cod)

MMSYSCODE mixerline_table[] =
{
 MMCODE(MIXERLINE_COMPONENTTYPE_DST_DIGITAL, 
	"digital destination (for example, digital input to a DAT or CD audio device)")
,MMCODE(MIXERLINE_COMPONENTTYPE_DST_HEADPHONES, 
	"Audio line is intended to drive headphones.")
,MMCODE(MIXERLINE_COMPONENTTYPE_DST_LINE, 
	"line level destination")
,MMCODE(MIXERLINE_COMPONENTTYPE_DST_MONITOR, 
		"destination used for a monitor.")
,MMCODE(MIXERLINE_COMPONENTTYPE_DST_SPEAKERS, 
		"Audio line is intended to drive speakers.")
,MMCODE(MIXERLINE_COMPONENTTYPE_DST_TELEPHONE, 
		"destination that will be routed to a telephone line.")
,MMCODE(MIXERLINE_COMPONENTTYPE_DST_UNDEFINED, 
		"destination that cannot be defined by one of the standard component types.")
,MMCODE(MIXERLINE_COMPONENTTYPE_DST_VOICEIN, 
		"destination that will be the final recording source for voice input.")
,MMCODE(MIXERLINE_COMPONENTTYPE_DST_WAVEIN, 
		"destination that will be the final recording source for the waveform-audio input (ADC)."
		"This line typically provides some sort of gain or attenuation. ")
,MMCODE(MIXERLINE_COMPONENTTYPE_SRC_ANALOG, 
		"Audio line is an analog source (for example, analog output from a video-cassette tape).")
,MMCODE(MIXERLINE_COMPONENTTYPE_SRC_AUXILIARY, 
		"source originating from the auxiliary audio line."
		"This line type is intended as a source with gain or attenuation that can be routed to the MIXERLINE_COMPONENTTYPE_DST_SPEAKERS"
		"destination and/or recorded from the MIXERLINE_COMPONENTTYPE_DST_WAVEIN destination.")
,MMCODE(MIXERLINE_COMPONENTTYPE_SRC_COMPACTDISC, 
		"source originating from the output of an internal audio CD."
		"This component type is provided for audio cards that provide an audio source line intended to be connected to an audio CD"
		" (or CD-ROM playing an audio CD).")
,MMCODE(MIXERLINE_COMPONENTTYPE_SRC_DIGITAL, 
		"digital source (for example, digital output from a DAT or audio CD).")
,MMCODE(MIXERLINE_COMPONENTTYPE_SRC_LINE, 
		"line-level source (for example, line-level input from an external stereo) that can be used as an optional recording source."
		"Because most audio cards for personal computers provide some sort of gain for the recording source line, the mixer device"
		" will use the MIXERLINE_COMPONENTTYPE_SRC_AUXILIARY type.")
,MMCODE(MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE, 
		"microphone recording source."
		"Most audio cards for personal computers provide at least two types of recording sources: an auxiliary audio line"
		"and microphone input. A microphone audio line typically provides some sort of gain."
		"Audio cards that use a single input for use with a microphone or auxiliary audio line should use "
		"the MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE component type.")
,MMCODE(MIXERLINE_COMPONENTTYPE_SRC_PCSPEAKER, 
		"source originating from personal computer speaker."
		"Several audio cards for personal computers provide the ability to mix what would typically be played on the internal "
		"speaker with the output of an audio card."
		"Some audio cards support the ability to use this output as a recording source.")
,MMCODE(MIXERLINE_COMPONENTTYPE_SRC_SYNTHESIZER, 
		"source originating from the output of an internal synthesizer."
		"Most audio cards for personal computers provide some sort of MIDI synthesizer (for example, an Adlib®-compatible "
		"or OPL/3 FM synthesizer).")
,MMCODE(MIXERLINE_COMPONENTTYPE_SRC_TELEPHONE, 
		"source originating from an incoming telephone line.")
,MMCODE(MIXERLINE_COMPONENTTYPE_SRC_UNDEFINED, 
		"source that cannot be defined by one of the standard component types."
		"A mixer device is required to use this component type for line component types that have not been "
		"defined by Microsoft Corporation.")
,MMCODE(MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT, 
		"source originating from the waveform-audio output digital-to-analog converter (DAC)."
		"Most audio cards for personal computers provide this component type as a source to the MIXERLINE_COMPONENTTYPE_DST_SPEAKERS destination. Some cards also allow this source to be routed to the "
		"MIXERLINE_COMPONENTTYPE_DST_WAVEIN destination.")
};

#define MXLINENAME(cod) MMNAME(mixerline_table,cod)
#define MXLINEDESC(cod) MMDESC(mixerline_table,cod)

const char * get_mmcode(MMSYSCODE table[], int nelems, int code)
{ int i;

	for (i = 0; i < nelems; i++) {
		if (table[i].code == code) {
			return table[i].name;
		}
	}
	return "UNDEFINED";
}

const char * get_mmmsg(MMSYSCODE table[], int nelems, int code)
{ int i;

	for (i = 0; i < nelems; i++) {
		if (table[i].code == code) {
			return table[i].msg;
		}
	}
	return "unknown";
}


void
dbprintf (const char * fmt, ...)
{ va_list varg;
  char buffer[1024];

	va_start(varg, fmt);
	_vsnprintf(buffer, sizeof(buffer), fmt, varg);
	va_end(varg);
#ifdef UNDER_CE
	WCHAR wbuffer[1024];
    MultiByteToWideChar(CP_ACP, 0, buffer, -1, wbuffer, NELEMS(wbuffer));
	OutputDebugString(wbuffer);
#else
	OutputDebugString(buffer);
#endif
}

void mmdiag(const char * fname, MMRESULT mr)
{
	dbprintf ("%s() returned %d - %s (%s)\n", fname, mr, MMERRNAME(mr), MMERRDESC(mr));
}

void
dump_mixercaps(const MIXERCAPS & mxCaps)
{
	dbprintf ("Mixer Caps:\n");
	dbprintf ("Manufacturer: %4d\n", mxCaps.wMid);
	dbprintf ("Product:      %4d\n", mxCaps.wPid);
    dbprintf ("Version:      %d.%d\n", HIBYTE(mxCaps.vDriverVersion),LOBYTE(mxCaps.vDriverVersion)); 
    dbprintf ("Name:         \"" SSTR "\"\n", mxCaps.szPname);
    dbprintf ("Support:      %x\n", mxCaps.fdwSupport);
    dbprintf ("Num. lines:   %d\n", mxCaps.cDestinations);
}



void
dump_mixerline (const MIXERLINE & mxLine)
{
//    dbprintf ("size:        %d\n", mxLine.cbStruct);
    dbprintf ("%08x: %2d -> %2d, %2d controls"
		, mxLine.dwLineID
		, mxLine.dwSource
		, mxLine.dwDestination
		, mxLine.cControls
		);
	if (!(mxLine.fdwLine & MIXERLINE_LINEF_SOURCE)) {
		dbprintf (", %2d connections ", mxLine.cConnections);
	}
	switch (mxLine.cChannels)
	{
	case 1:
		dbprintf (", mono\n");
		break;
	case 2:
		dbprintf (", stereo\n");
		break;
	default:
		dbprintf (", %d channels\n");
		break;
	}
    dbprintf ("  name:  \"" SSTR "\", \"" SSTR "\"\n"
		, mxLine.szShortName 
		, mxLine.szName
		);
    dbprintf ("  flags:  %08x ", mxLine.fdwLine);
	if (mxLine.fdwLine & MIXERLINE_LINEF_SOURCE) {
		dbprintf ("src");
	}
	else {
		dbprintf ("dst");
	}
	if (mxLine.fdwLine & MIXERLINE_LINEF_ACTIVE) {
		dbprintf (" active");
	}
	if (!(mxLine.fdwLine & MIXERLINE_LINEF_DISCONNECTED)) {
		dbprintf (" connected");
	}
	dbprintf ("\n");

//    dbprintf ("user:   %x\n", mxLine.dwUser);
    dbprintf ("  type:   %s (%08x)\n", MXLINENAME(mxLine.dwComponentType), mxLine.dwComponentType);
	dbprintf ("  target: %8x - \"" SSTR "\"\n"
		, mxLine.Target.dwDeviceID
		, mxLine.Target.szPname
		);
}

void
dump_control (HMIXEROBJ hmx, const MIXERCONTROL & mxControl, DWORD cChannels)
{ const char * control_type;
  MIXERCONTROLDETAILS mxDetails;
  MIXERCONTROLDETAILS_LISTTEXT ulTitles[10];
  MMRESULT mr;

	switch (mxControl.dwControlType) {
#define MXCTYPE(t) case MIXERCONTROL_CONTROLTYPE_##t: control_type = #t; break;
	MXCTYPE(CUSTOM);
	MXCTYPE(BASS);
	MXCTYPE(EQUALIZER);
	MXCTYPE(FADER);
	MXCTYPE(TREBLE);
	MXCTYPE(VOLUME);
	MXCTYPE(MIXER);
	MXCTYPE(MULTIPLESELECT);
	MXCTYPE(MUX);
	MXCTYPE(SINGLESELECT);
	MXCTYPE(BOOLEANMETER);
	MXCTYPE(PEAKMETER);
	MXCTYPE(SIGNEDMETER);
	MXCTYPE(UNSIGNEDMETER);
	MXCTYPE(DECIBELS);
	MXCTYPE(PERCENT);
	MXCTYPE(SIGNED);
	MXCTYPE(UNSIGNED);
	MXCTYPE(PAN);
	MXCTYPE(QSOUNDPAN);
	MXCTYPE(SLIDER);
	MXCTYPE(BOOLEAN);
	MXCTYPE(BUTTON);
	MXCTYPE(LOUDNESS);
	MXCTYPE(MONO);
	MXCTYPE(MUTE);
	MXCTYPE(ONOFF);
	MXCTYPE(STEREOENH);
	MXCTYPE(MICROTIME);
	MXCTYPE(MILLITIME);
	default:
		control_type = "unknown";
		break;
	}

	dbprintf ("     %08x \"" SSTR "\" \"" SSTR "\" %s (%08x) ["
		, mxControl.dwControlID
		, mxControl.szShortName
		, mxControl.szName
		, control_type
		, mxControl.dwControlType
		);
	if (mxControl.fdwControl & MIXERCONTROL_CONTROLF_DISABLED) {
		dbprintf (" disabled");
	}
	if (mxControl.fdwControl & MIXERCONTROL_CONTROLF_MULTIPLE) {
		dbprintf (" multiple");
	}
	if (mxControl.fdwControl & MIXERCONTROL_CONTROLF_UNIFORM) {
		dbprintf (" uniform");
	}
	dbprintf ("] %d\n", mxControl.cMultipleItems);

	mxDetails.cbStruct = sizeof(mxDetails);
	mxDetails.dwControlID = mxControl.dwControlID;
	if (mxControl.fdwControl & MIXERCONTROL_CONTROLF_UNIFORM) {
		mxDetails.cChannels = 1;
	}
	else {
		mxDetails.cChannels = cChannels;
	}
	if (mxControl.fdwControl & MIXERCONTROL_CONTROLF_MULTIPLE) {
		mxDetails.cMultipleItems = mxControl.cMultipleItems;
	}
	else {
		mxDetails.cMultipleItems = 0;
	}

	if (mxControl.fdwControl & MIXERCONTROL_CONTROLF_MULTIPLE) {
		mxDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_LISTTEXT);
		mxDetails.paDetails = ulTitles;
		mr = mixerGetControlDetails (hmx, &mxDetails, MIXER_GETCONTROLDETAILSF_LISTTEXT | MIXER_OBJECTF_HMIXER);
		if (mr != MMSYSERR_NOERROR) {
			mmdiag("mixerGetControlDetails", mr);
			return;
		}
		for (UINT c = 0; c < mxDetails.cMultipleItems; c++) {
			dbprintf("     %d: %8x %8x \"%4s\"\n"
				, c
				, ulTitles[c].dwParam1
				, ulTitles[c].dwParam2
				, ulTitles[c].szName
				);
		}
	}

	switch (mxControl.dwControlType) {
	case MIXERCONTROL_CONTROLTYPE_VOLUME:
		{ MIXERCONTROLDETAILS_UNSIGNED ulValues[10];
			mxDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);
			mxDetails.paDetails = ulValues;
			mr = mixerGetControlDetails (hmx, &mxDetails, MIXER_GETCONTROLDETAILSF_VALUE | MIXER_OBJECTF_HMIXER);
			if (mr != MMSYSERR_NOERROR) {
				mmdiag("mixerGetControlDetails", mr);
				return;
			}
			dbprintf ("    settings:");
			for (UINT c = 0; c < cChannels; c++) {
				dbprintf(" %4d", ulValues[c]);
			}
			dbprintf ("\n");
		}
		break;

	case MIXERCONTROL_CONTROLTYPE_MIXER:
	case MIXERCONTROL_CONTROLTYPE_MULTIPLESELECT:
	case MIXERCONTROL_CONTROLTYPE_SINGLESELECT:
	case MIXERCONTROL_CONTROLTYPE_MUX:
		{ MIXERCONTROLDETAILS_BOOLEAN ulValues[10];

			if (!(mxControl.fdwControl & MIXERCONTROL_CONTROLF_MULTIPLE)) {
				dbprintf ("weird control: list class but not multiple items !?\n");
				//return;
			}
			mxDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
			mxDetails.paDetails = ulValues;
			mr = mixerGetControlDetails (hmx, &mxDetails, MIXER_GETCONTROLDETAILSF_VALUE | MIXER_OBJECTF_HMIXER);
			if (mr != MMSYSERR_NOERROR) {
				mmdiag("mixerGetControlDetails", mr);
				return;
			}
			for (UINT c = 0; c < mxDetails.cMultipleItems; c++) {
				if (ulValues[c].fValue) {
					dbprintf("     selecting [%d] - \"%4s\"\n"
						, c
						, ulTitles[c].szName
						);
				}
			}
		}
		break;

	case MIXERCONTROL_CONTROLTYPE_MUTE:
		{ MIXERCONTROLDETAILS_UNSIGNED ulValues[10];
			mxDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);
			mxDetails.paDetails = ulValues;
			mr = mixerGetControlDetails (hmx, &mxDetails, MIXER_GETCONTROLDETAILSF_VALUE | MIXER_OBJECTF_HMIXER);
			if (mr != MMSYSERR_NOERROR) {
				mmdiag("mixerGetControlDetails", mr);
				return;
			}
			dbprintf ("    settings:");
			for (UINT c = 0; c < cChannels; c++) {
				dbprintf(" %4d", ulValues[c]);
			}
			dbprintf ("\n");
		}
		break;

	case MIXERCONTROL_CONTROLTYPE_CUSTOM:
	case MIXERCONTROL_CONTROLTYPE_BASS:
	case MIXERCONTROL_CONTROLTYPE_EQUALIZER:
	case MIXERCONTROL_CONTROLTYPE_FADER:
	case MIXERCONTROL_CONTROLTYPE_TREBLE:

	case MIXERCONTROL_CONTROLTYPE_BOOLEANMETER:
	case MIXERCONTROL_CONTROLTYPE_PEAKMETER:
	case MIXERCONTROL_CONTROLTYPE_SIGNEDMETER:
	case MIXERCONTROL_CONTROLTYPE_UNSIGNEDMETER:
	case MIXERCONTROL_CONTROLTYPE_DECIBELS:
	case MIXERCONTROL_CONTROLTYPE_PERCENT:
	case MIXERCONTROL_CONTROLTYPE_SIGNED:
	case MIXERCONTROL_CONTROLTYPE_UNSIGNED:
	case MIXERCONTROL_CONTROLTYPE_PAN:
	case MIXERCONTROL_CONTROLTYPE_QSOUNDPAN:
	case MIXERCONTROL_CONTROLTYPE_SLIDER:
	case MIXERCONTROL_CONTROLTYPE_BOOLEAN:
	case MIXERCONTROL_CONTROLTYPE_BUTTON:
	case MIXERCONTROL_CONTROLTYPE_LOUDNESS:
	case MIXERCONTROL_CONTROLTYPE_MONO:
	case MIXERCONTROL_CONTROLTYPE_ONOFF:
	case MIXERCONTROL_CONTROLTYPE_STEREOENH:
	case MIXERCONTROL_CONTROLTYPE_MICROTIME:
	case MIXERCONTROL_CONTROLTYPE_MILLITIME:
	default:
		break;
	}
}

void
probe_controls (HMIXEROBJ hmxo, const MIXERLINE & mxLine)
{ MIXERLINECONTROLS mxLineControls;
  MIXERCONTROL mxControls[20];
  MMRESULT mr;
  UINT controlnum;

	if (mxLine.cControls == 0) {
		dbprintf ("line has no controls\n");
		return;
	}
	if (mxLine.cControls > NELEMS(mxControls)) {
		dbprintf ("line has too many controls (%d) to handle\n", mxLine.cControls);
		return;
	}
	mxLineControls.cbStruct = sizeof(mxLineControls);
	mxLineControls.cControls = mxLine.cControls;
	mxLineControls.dwLineID = mxLine.dwLineID;
	mxLineControls.cbmxctrl = sizeof(mxControls[0]);
	mxLineControls.pamxctrl = mxControls;
	mr = mixerGetLineControls (hmxo, &mxLineControls,  MIXER_GETLINECONTROLSF_ALL | MIXER_OBJECTF_HMIXER);
	if (mr != MMSYSERR_NOERROR) {
		mmdiag("mixerGetLineControls", mr);
		return;
	}
	for ( controlnum = 0; controlnum < mxLineControls.cControls; controlnum++) {
		dump_control(hmxo, mxControls[controlnum], mxLine.cChannels);
	}
}

void
play_with_mixer (HMIXEROBJ mxho)
{ MMRESULT mr;
  MIXERCAPS mxCaps;
  MIXERLINE mxLine;
  UINT destnum;
  UINT srcnum;

	dbprintf ("Querying device caps\n");
	mr = mixerGetDevCaps((UINT)mxho, &mxCaps, sizeof(mxCaps));
	if (mr != MMSYSERR_NOERROR) {
		mmdiag("mixerGetDevCaps", mr);
		return;
	}
	dbprintf ("===========================================================\n");
	dump_mixercaps(mxCaps);
	dbprintf ("===========================================================\n");
	for (destnum = 0; destnum < mxCaps.cDestinations; destnum++) {
		dbprintf ("Querying destination line %d\n", destnum);
		mxLine.cbStruct = sizeof(mxLine);
		mxLine.dwDestination = destnum;
		mxLine.dwSource = 0; // BUGBUG - the driver is anal about this...
		mr = mixerGetLineInfo(mxho, &mxLine, MIXER_GETLINEINFOF_DESTINATION | MIXER_OBJECTF_HMIXER);
		if (mr != MMSYSERR_NOERROR) {
			mmdiag("mixerGetLineInfo", mr);
			continue;
		}
		dump_mixerline(mxLine);
		probe_controls(mxho, mxLine);
		dbprintf ("Source lines for \"" SSTR "\"\n", mxLine.szName, destnum);
		for (srcnum = 0; srcnum < mxLine.cConnections; srcnum++) {
			MIXERLINE mxSource;
			mxSource.cbStruct = sizeof(mxSource);
			mxSource.dwDestination = destnum;
			mxSource.dwSource = srcnum;
			mr = mixerGetLineInfo(mxho, &mxSource, MIXER_GETLINEINFOF_SOURCE | MIXER_OBJECTF_HMIXER);
			if (mr != MMSYSERR_NOERROR) {
				mmdiag("mixerGetLineInfo", mr);
				continue;
			}
			dump_mixerline(mxSource);
			probe_controls(mxho, mxSource);
		}
	dbprintf ("-----------------------------------------------------------\n");
 	}
}



int WINAPI
WinMain(
    HINSTANCE /* hInstance*/,	// handle to current instance
    HINSTANCE /* hPrevInstance */,	// handle to previous instance
    LPTSTR /* lpCmdLine */,	// pointer to command line
    int /* nCmdShow */ 	// show state of window
   )
{ MMRESULT mr;
  HMIXER mxh;
  int num_mixer_devs;
  int devnum;
  UINT devid;

	dbprintf ("TMIX.EXE: Mixer API Test Application Version 1.0, Built: %s\n", __DATE__);
	dbprintf ("mixerGetNumDevs...\n");
	num_mixer_devs = mixerGetNumDevs();
	dbprintf ("Found %d mixer device%s\n", num_mixer_devs, num_mixer_devs == 1 ? "" : "s");
	for (devnum = 0; devnum <= num_mixer_devs; devnum++) {
		dbprintf ("Trying device %d of %d\n", devnum, num_mixer_devs);
		dbprintf ("Querying ID for device with mixerGetID\n");
		mr = mixerGetID ((HMIXEROBJ)devnum, &devid,  MIXER_OBJECTF_MIXER);
		if (mr != MMSYSERR_NOERROR) {
			mmdiag ("mixerGetID", mr);
		}
		dbprintf ("Opening device with mixerOpen\n");
		mr = mixerOpen (&mxh, devnum, 0, 0, MIXER_OBJECTF_MIXER);
		if (mr != MMSYSERR_NOERROR) {
			mmdiag("mixerOpen", mr);
			// deliberately let a bad handle fall through to exercise failure code below
			//continue;
		}
		else {
			dbprintf ("mixerOpen() OK\n");
			play_with_mixer((HMIXEROBJ) mxh);
		}
		mr = mixerClose(mxh);
		if (mr != MMSYSERR_NOERROR) {
			mmdiag ("mixerClose", mr);
		}
		else {
			dbprintf ("mixerClose() OK\n");
		}
	}
	dbprintf ("TMIX.EXE: done\n");
	return 0;
}