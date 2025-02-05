#include <windows.h>
#include <ceconfig.h>

typedef HRESULT (WINAPI _ATL_CREATORFUNC)(void* pv, REFIID riid, LPVOID* ppv);
typedef LPCTSTR (WINAPI _ATL_DESCRIPTIONFUNC)();

namespace ATL
{
    struct _ATL_OBJMAP_ENTRY
    {
        const CLSID* pclsid;
        HRESULT (WINAPI *pfnUpdateRegistry)(BOOL bRegister);
        _ATL_CREATORFUNC* pfnGetClassObject;
        _ATL_CREATORFUNC* pfnCreateInstance;
        IUnknown* pCF;
        DWORD dwRegister;
        _ATL_DESCRIPTIONFUNC* pfnGetObjectDescription;
        HRESULT WINAPI RevokeClassObject()
        {
            return E_NOTIMPL;
        }
        HRESULT WINAPI RegisterClassObject(DWORD dwClsContext, DWORD dwFlags)
        {
            return E_NOTIMPL;
        }
    };
};

#define BEGIN_OBJECT_MAP(x) extern ATL::_ATL_OBJMAP_ENTRY x[] = {
#define END_OBJECT_MAP()   {NULL, NULL, NULL, NULL}};
#define OBJECT_ENTRY(clsid, classname) \
    {&clsid, NULL, classname##::s_pfnGetClassObject, classname##::s_pfnCreateInstance, NULL, 0, NULL},

#define DECLAREOBJ(classname) \
class classname## \
{ \
public: \
    static _ATL_CREATORFUNC* s_pfnGetClassObject; \
	static _ATL_CREATORFUNC* s_pfnCreateInstance; \
};

DECLAREOBJ(CdxmPlay);
DECLAREOBJ(CAMtoolbar);
DECLAREOBJ(CDSStatusBar);
DECLAREOBJ(CDSDisplayPanel);
DECLAREOBJ(CDSGotoBar);
DECLAREOBJ(CDSAdBanner);

EXTERN_C const CLSID \
    CLSID_MediaPlayer, \
    CLSID_NSPlay, \
    CLSID_ActiveMovie, \
    CLSID_AMtoolbar, \
    CLSID_DSStatusBar, \
    CLSID_DSDisplayPanel,\
    CLSID_DSGotoBar,\
    CLSID_DSAdBanner;

BEGIN_OBJECT_MAP(ObjectMap)

    OBJECT_ENTRY(CLSID_MediaPlayer, CdxmPlay)
#ifdef DIRECTX_MODULES_NSCOMPAT
    {&CLSID_NSPlay, NULL, CdxmPlay::s_pfnGetClassObject, (_ATL_CREATORFUNC*)1, NULL, 0, NULL},
#endif
#ifdef DIRECTX_MODULES_AMCOMPAT
    {&CLSID_ActiveMovie, NULL, CdxmPlay::s_pfnGetClassObject, (_ATL_CREATORFUNC*)2, NULL, 0, NULL},
#endif

    OBJECT_ENTRY(CLSID_AMtoolbar, CAMtoolbar)
#ifdef MSDXM_STATBAR
    OBJECT_ENTRY(CLSID_DSStatusBar, CDSStatusBar)
#endif
#ifdef MSDXM_DISPBAR
    OBJECT_ENTRY(CLSID_DSDisplayPanel, CDSDisplayPanel)
#endif
#ifdef MSDXM_GOTOBAR
    OBJECT_ENTRY(CLSID_DSGotoBar, CGotoBar)
#endif
#ifdef MSDXM_ADBAR
    OBJECT_ENTRY(CLSID_DSAdBanner, CAdBanner)
#endif
#ifdef MSDXM_RADIOBAR
    OBJECT_ENTRY(CLSID_AsyncPProt, CAsyncPProt)
    OBJECT_ENTRY(CLSID_AsyncMHandler, CAsyncMHandler)
    OBJECT_ENTRY(CLSID_RadioPlayer, CRadioPlayer)
    OBJECT_ENTRY(CLSID_RadioServer, CRadioServer)
    OBJECT_ENTRY(CLSID_RadioBand, CRadioBand)
    OBJECT_ENTRY(CLSID_RadioView, CRadioView)
#endif

END_OBJECT_MAP()

