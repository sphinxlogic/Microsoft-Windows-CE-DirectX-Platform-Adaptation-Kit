#ifndef __DDPGUIDS_H__
#define __DDPGUIDS_H__

#ifdef __cplusplus
extern "C" {
#endif

DEFINE_GUID( GUID_HALMemory, 	  0xb4ed0de6, 0xfa3f, 0x11d0, 0xb7, 0x74, 0x0, 0xc0, 0x4f, 0xc3, 0x21, 0x84 );

typedef struct _DDHAL_HALGETVIDMEMDATA FAR *LPDDHAL_HALGETVIDMEMDATA;
typedef struct _DDHAL_HALSETSURFACEDESCDATA FAR *LPDDHAL_HALSETSURFACEDESCDATA;


/*
 * DDHALMEM callbacks
 */
typedef DWORD (FAR PASCAL *LPDDHALMEMCB_HALGETVIDMEM)(LPDDHAL_HALGETVIDMEMDATA);
typedef DWORD (FAR PASCAL *LPDDHALMEMCB_HALSETSURFACEDESC)(LPDDHAL_HALSETSURFACEDESCDATA);


typedef struct _DDHAL_DDHALMEMORYCALLBACKS
{
    DWORD                               dwSize;
    DWORD                               dwFlags;
    LPDDHALMEMCB_HALGETVIDMEM           HalGetVidMem;
	LPDDHALMEMCB_HALSETSURFACEDESC		HalSetSurfaceDesc;
} DDHAL_DDHALMEMORYCALLBACKS;
typedef DDHAL_DDHALMEMORYCALLBACKS FAR *LPDDHAL_DDHALMEMORYCALLBACKS;

#define DDHAL_KERNEL_HALGETVIDMEM		0x00000001l
#define DDHAL_KERNEL_HALSETSURFACEDESC	0x00000002l


/*
 * structure for passing information to DDHAL HalGetVidMem fn
 */
typedef struct _DDHAL_HALGETVIDMEMDATA
{
    DWORD       	dwSize;                 // Size of this structure
	LPDDSCAPS		lpDDSCaps;				// Hardware caps of proposed surface
	LPDWORD			lpdwTotal;				// Total amount of this memory type
	LPDWORD			lpdwFree;				// Available amount of this memory type
    HRESULT     	ddrval;
} DDHAL_HALGETVIDMEMDATA;

/*
 * structure for passing information to DDHAL HalSetSurfaceDesc fn
 */
typedef struct _DDHAL_HALSETSURFACEDESCDATA
{
    DWORD       	dwSize;             	// Size of this structure
    LPDDRAWI_DDRAWSURFACE_LCL  lpDDSurface; // Surface
	LPDDSURFACEDESC	lpddsd;					// Description of surface
    HRESULT     	ddrval;
} DDHAL_HALSETSURFACEDESCDATA;

#ifdef __cplusplus
};
#endif


#endif // __DDPGUIDS_H__
