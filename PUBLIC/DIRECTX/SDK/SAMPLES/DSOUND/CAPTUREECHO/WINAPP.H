#pragma once
#ifndef __WINAPP_H__
#define __WINAPP_H__

class CWindowsModule
{
public:
    static CWindowsModule& GetWinModule();
    static CWindowsModule* GetWinModulePtr();

    CWindowsModule();
    virtual ~CWindowsModule();

    virtual bool Initialize();
    virtual void Shutdown();

#   ifdef UNDER_CE
    typedef LPWSTR command_line_type;
#   else
    typedef LPSTR command_line_type;
#   endif

    const HINSTANCE m_hInstance;

    const command_line_type m_pszCommandLine;
    HWND m_hWnd;
    LPCTSTR m_pszModuleName;
    INT m_iAppReturnCode;

protected:
    virtual bool InitConsoleOutput();

    virtual bool RegisterWindowClass();
    virtual void UnregisterWindowClass();

    virtual bool CreateMainWindow();
    virtual void DestroyMainWindow();

    WNDCLASS m_WindowClass;

public:
    static BOOL DllMain(HANDLE hInstance, ULONG dwReason, LPVOID lpReserved);
};

class CWindowsApplication : public CWindowsModule
{
public:
    static CWindowsApplication& GetWinApp();
    static CWindowsApplication* GetWinAppPtr();

    CWindowsApplication();
    virtual ~CWindowsApplication();

    virtual void Run() = 0; 
    virtual void Shutdown();

public:
    static int WinMain(HINSTANCE hInstance, HINSTANCE, command_line_type pszCmdLine, int);
};


#endif