#include "main.h"
#include "WinApp.h"

namespace 
{
    CWindowsModule *l_pWindowsModule = NULL;
    CWindowsApplication *l_pWindowsApplication = NULL;
}

CWindowsModule::CWindowsModule()
: m_hInstance(NULL), 
  m_hWnd(NULL), 
  m_pszModuleName(NULL), 
  m_iAppReturnCode(0),
  m_pszCommandLine(NULL)
{
    WNDCLASS& wc = m_WindowClass;

    // Window Class defaults
    wc.style            = 0;
    wc.lpfnWndProc      = DefWindowProc;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;
    wc.hIcon            = NULL;
    wc.hCursor          = NULL;
    wc.hbrBackground    = NULL;
    wc.lpszMenuName     = NULL;
    wc.lpszClassName    = TEXT("CWindowsModule_WindowClass");

    ASSERT(l_pWindowsModule==NULL);
    l_pWindowsModule = this;
}


CWindowsModule::~CWindowsModule()
{
    l_pWindowsModule = NULL;
}

bool CWindowsModule::InitConsoleOutput()
{
    return true;
}

CWindowsModule& CWindowsModule::GetWinModule()
{
    ASSERT(l_pWindowsModule!=NULL);
    return *l_pWindowsModule;
}

CWindowsModule* CWindowsModule::GetWinModulePtr()
{
    return l_pWindowsModule;
}

bool CWindowsModule::RegisterWindowClass()
{
    m_WindowClass.hInstance = m_hInstance;
    return RegisterClass(&m_WindowClass)!=0;
}

void CWindowsModule::UnregisterWindowClass()
{
    UnregisterClass(m_WindowClass.lpszClassName, m_hInstance);
}

bool CWindowsModule::CreateMainWindow()
{
    m_hWnd = CreateWindow(m_WindowClass.lpszClassName,m_pszModuleName,0,0,0,0,0,NULL,NULL,m_hInstance,NULL);
    return m_hWnd!=NULL;
}

void CWindowsModule::DestroyMainWindow()
{
    if (m_hWnd) DestroyWindow(m_hWnd);
    m_hWnd = NULL;
}

bool CWindowsModule::Initialize()
{
    return RegisterWindowClass() && CreateMainWindow();
}

void CWindowsModule::Shutdown()
{
    DestroyMainWindow();
    UnregisterWindowClass();
    const_cast<HINSTANCE&>(l_pWindowsModule->m_hInstance) = NULL;
}


CWindowsApplication::CWindowsApplication()
{
    ASSERT(l_pWindowsApplication==NULL);
    l_pWindowsApplication = this;
}

CWindowsApplication::~CWindowsApplication()
{
    l_pWindowsApplication = NULL;
}

CWindowsApplication& CWindowsApplication::GetWinApp()
{
    ASSERT(l_pWindowsApplication!=NULL);
    return *l_pWindowsApplication;
}

CWindowsApplication* CWindowsApplication::GetWinAppPtr()
{
    return l_pWindowsApplication;
}

void CWindowsApplication::Shutdown()
{
    MSG msg;

    // Pump any remaining messages
    while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    DestroyMainWindow();

    // Pump any other messages
    while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnregisterWindowClass();
}

int CWindowsApplication::WinMain(HINSTANCE hInstance, HINSTANCE, command_line_type pszCmdLine, int)
{
    if (l_pWindowsApplication==NULL)
    {
        OutputDebugString(TEXT("SERIOUS FAILURE: No instance of a Windows Application class has been defined.\n"));
        OutputDebugString(TEXT("Terminating application.\n"));
        return -1;
    }

    l_pWindowsApplication->InitConsoleOutput();

    const_cast<HINSTANCE&>(l_pWindowsApplication->m_hInstance) = hInstance;
    const_cast<command_line_type>(l_pWindowsApplication->m_pszCommandLine) = pszCmdLine;

    if (l_pWindowsApplication->Initialize())
        l_pWindowsApplication->Run();
    l_pWindowsApplication->Shutdown();

    return l_pWindowsApplication->m_iAppReturnCode;
}

BOOL CWindowsModule::DllMain(HANDLE hInstance, ULONG dwReason, LPVOID lpReserved)
{
    if (l_pWindowsModule==NULL)
    {
        OutputDebugString(TEXT("SERIOUS FAILURE: No instance of a Windows Module class has been defined.\n"));
        OutputDebugString(TEXT("Terminating application.\n"));
        return FALSE;
    }

    switch (dwReason) 
    {
    case DLL_PROCESS_ATTACH:
        l_pWindowsModule->InitConsoleOutput();

        // Initialize a few variables
        const_cast<HINSTANCE&>(l_pWindowsModule->m_hInstance) = (HINSTANCE)hInstance;

        if (!l_pWindowsModule->Initialize())
            return FALSE;
        break;

    case DLL_PROCESS_DETACH:
        l_pWindowsModule->Shutdown();
        break;
    }

    return TRUE;
}
