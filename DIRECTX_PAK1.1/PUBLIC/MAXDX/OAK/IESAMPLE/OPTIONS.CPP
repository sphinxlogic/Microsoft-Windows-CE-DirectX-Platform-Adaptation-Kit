/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.
Copyright (c) 1998-2000  Microsoft Corporation

Module Name:  options.cpp

Abstract:  Implements the Internet Options dialog
  
Functions:

Notes: 

--*/
#include "Precomp.h"
#include "resource.h"
#include "Mainwnd.h"

#if !defined(NOCOMMANDBAR) && !defined(NOOPTIONS)

#define MAX_REG_VALUE_LEN	50

static TCHAR *pszKey = _T("Software\\Microsoft\\Internet Explorer\\Main");
static TCHAR *pszCacheKey = _T("Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings\\Cache\\Content");
static TCHAR *pszAutoDialKey = _T("RemoteAccess");
static TCHAR *pszOptions[3] = {_T("no"), _T("yes"), _T("hover")};
static TCHAR *pszUseLanNoAutodial = _T("Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings");

void SaveOptionsToRegistry(HWND hDlg);
void LoadOptionsFromRegistry(HWND hDlg);

BOOL CALLBACK CMainWnd::OptionsDlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
	case WM_INITDIALOG:
		LoadOptionsFromRegistry(hwnd);
		break;
	case WM_COMMAND:
	{
		if ((HIWORD(wParam) == BN_CLICKED))
		{
			if (LOWORD(wParam) == IDC_USEPROXY)
			{
				BOOL bEnable = SendMessage(((HWND )lParam), BM_GETCHECK, 0L, 0L);
				EnableWindow(GetDlgItem(hwnd, IDC_ADDRESS), bEnable);
				EnableWindow(GetDlgItem(hwnd, IDC_PORT), bEnable);
				return TRUE;
			}
			else if (LOWORD(wParam) == IDOK)
			{
				SaveOptionsToRegistry(hwnd);
				EndDialog(hwnd, TRUE);
				return TRUE;
			}
			else if (LOWORD(wParam) == IDCANCEL)
			{
				EndDialog(hwnd, FALSE);
				return TRUE;
			}
		}
	}
		break;

	default:
		break;
	}
	return FALSE;
}

void SaveOptionsToRegistry(HWND hDlg)
{
	HKEY hKey = NULL;
	DWORD dwDisp;
	TCHAR szString[MAXLEN];

	//start page, search page, underline anchors
	LONG lRes = RegCreateKeyEx(HKEY_CURRENT_USER, pszKey, 0, NULL, 0, 0, 0, &hKey, &dwDisp);
	if (hKey && lRes==ERROR_SUCCESS)
	{
		GetDlgItemText(hDlg, IDC_START, szString, MAXLEN);
		lRes = RegSetValueEx(hKey, _T("Start Page"), 0, REG_SZ, (BYTE *)szString, MAXLEN);

		GetDlgItemText(hDlg, IDC_SEARCH, szString, MAXLEN);
		lRes = RegSetValueEx(hKey, _T("Search Page"), 0, REG_SZ, (BYTE *)szString, MAXLEN);

		for(UINT nID=IDC_UNDERLINENEVER; nID<=IDC_UNDERLINEHOVER; nID++)
		{
			DWORD dwState = SendMessage(GetDlgItem(hDlg, nID), BM_GETCHECK, 0L, 0L);
			if (dwState == BST_CHECKED)
			{
				int nCheck;

				nCheck = nID-IDC_UNDERLINENEVER;
				_tcscpy(szString, pszOptions[nCheck]);
				lRes = RegSetValueEx(hKey, _T("Anchor Underline"), 0, REG_SZ, 
									(BYTE *)szString, MAX_REG_VALUE_LEN);

				break;
			}
		}
		RegCloseKey(hKey);
	}

	//cache size
	hKey = NULL;
	lRes = RegCreateKeyEx(HKEY_CURRENT_USER, pszCacheKey, 0, NULL, 0, 0, 0, &hKey, &dwDisp);
	if (hKey && lRes==ERROR_SUCCESS)
	{
		BOOL bTrans;
		DWORD dwVal = GetDlgItemInt(hDlg, IDC_CACHE, &bTrans, FALSE);
		if (bTrans)
		{
			lRes = RegSetValueEx(hKey, _T("CacheLimit"), 0, REG_DWORD, 
								 (BYTE *)(&dwVal), sizeof(DWORD));
		}

		RegCloseKey(hKey);
	}

	//ras autodial
	hKey = NULL;
	lRes = RegCreateKeyEx(HKEY_CURRENT_USER, pszAutoDialKey, 0, NULL, 0, 0, 0, &hKey, &dwDisp);
	if (hKey && lRes==ERROR_SUCCESS)
	{
		HWND hRasCombo = GetDlgItem(hDlg, IDC_AUTODIAL);
		int nIndex = SendMessage(hRasCombo, CB_GETCURSEL, 0, 0L);
		if (nIndex != CB_ERR)
		{		
			SendMessage(hRasCombo, CB_GETLBTEXT, (WPARAM )nIndex, (LPARAM )szString);
			TCHAR szRas[RAS_MaxEntryName + 1];
			_tcsncpy(szRas, szString, RAS_MaxEntryName);
			lRes = RegSetValueEx(hKey, _T("InternetProfile"), 0, REG_SZ, 
								(BYTE *)szRas, RAS_MaxEntryName*sizeof(TCHAR));
		}	
		RegCloseKey(hKey);
	}

	hKey = NULL;
	lRes = RegCreateKeyEx(HKEY_CURRENT_USER, pszUseLanNoAutodial, 0, NULL, 0, 0, 0, &hKey, &dwDisp);
	if (hKey && lRes==ERROR_SUCCESS)
	{
		DWORD dwVal = (SendMessage(GetDlgItem(hDlg, IDC_LANNOAUTODIAL), BM_GETCHECK, 0, 0L) == BST_CHECKED ? 0 : 1);
		lRes = RegSetValueEx(hKey, _T("EnableAutodial"), 0, REG_DWORD, (BYTE *)&dwVal, sizeof(dwVal));
		RegCloseKey(hKey);
	}

	//proxy info
	INTERNET_PROXY_INFO ipi;
	TCHAR szProxy[MAXLEN];
	UINT	cch;

	cch = GetDlgItemText(hDlg, IDC_ADDRESS, szProxy, MAXLEN);
	GetDlgItemText(hDlg, IDC_PORT, szString, MAXLEN);

	// prevent buffer overrun
	if (cch < (MAXLEN-1))
	{
		_tcsncat(szProxy, _T(":"), MAXLEN-1-cch);
		cch += 1;
	}
	if (cch < (MAXLEN-1))
	{
		_tcsncat(szProxy, szString, MAXLEN-1-cch);
	}

	ipi.lpszProxy = szProxy;

	ULONG uSize = sizeof(ipi);

	ipi.dwAccessType = (SendMessage(GetDlgItem(hDlg, IDC_USEPROXY), BM_GETCHECK, 0L, 0L)) ?
				INTERNET_OPEN_TYPE_PROXY : INTERNET_OPEN_TYPE_DIRECT;


	ipi.lpszProxyBypass = NULL;
	lRes = InternetSetOption(NULL, INTERNET_OPTION_PROXY, (LPVOID )(&ipi), uSize);

	SendNotifyMessage(HWND_BROADCAST, WM_SETTINGCHANGE, NULL, NULL);
}

void LoadOptionsFromRegistry(HWND hDlg)
{
	DWORD dwLen = MAXLEN, dwType = 0;
	HKEY hKey = NULL;
	TCHAR szString[MAXLEN];

	//start page, search page, underline anchors
	LONG lRes = RegOpenKeyEx(HKEY_CURRENT_USER, pszKey, 0, KEY_QUERY_VALUE, &hKey);
	if (hKey && lRes==ERROR_SUCCESS)
	{
		DWORD dwType;
		lRes = RegQueryValueEx(hKey, _T("Start Page"), 0, &dwType, (BYTE *)szString, &dwLen);
		SendMessage(GetDlgItem(hDlg, IDC_START), EM_SETLIMITTEXT, MAXLEN, 0);
		SetDlgItemText(hDlg, IDC_START, szString);

		dwLen = MAXLEN;
		lRes = RegQueryValueEx(hKey, _T("Search Page"), 0, &dwType, (BYTE *)szString, &dwLen);
		SendMessage(GetDlgItem(hDlg, IDC_SEARCH), EM_SETLIMITTEXT, MAXLEN, 0);
		SetDlgItemText(hDlg, IDC_SEARCH, szString);

		dwLen = MAXLEN;
		lRes = RegQueryValueEx(hKey, _T("Anchor Underline"), 0, &dwType, (BYTE *)szString, &dwLen);
		UINT nID = IDC_UNDERLINENEVER;
		while(nID <= IDC_UNDERLINEHOVER)
		{
			SendMessage(GetDlgItem(hDlg, nID), BM_SETCHECK, 0L, 0L);
			nID++;
		}
		nID = (!_tcscmp(szString, _T("no"))) ? 0 : 
					(!_tcscmp(szString, _T("yes"))) ? 1 : 2;
		nID += IDC_UNDERLINENEVER;
		SendMessage(GetDlgItem(hDlg, nID), BM_SETCHECK, TRUE, 0L);
		RegCloseKey(hKey);
	}

	//cache size

	// 4 chars allows 9999 KB which is > 9 GB
	SendMessage(GetDlgItem(hDlg, IDC_CACHE), EM_SETLIMITTEXT, 4, 0);

	hKey = NULL;
	lRes = RegOpenKeyEx(HKEY_CURRENT_USER, pszCacheKey, 0, KEY_QUERY_VALUE, &hKey);
	if (hKey && lRes==ERROR_SUCCESS)
	{
		DWORD dwVal;
		dwLen = sizeof(dwVal);
		lRes = RegQueryValueEx(hKey, _T("CacheLimit"), 0, &dwType, (BYTE *)(&dwVal), &dwLen);
		if (lRes == ERROR_SUCCESS)
		{
			SetDlgItemInt(hDlg, IDC_CACHE, dwVal, FALSE);
		}

		RegCloseKey(hKey);
	}


	//ras configs
	DWORD cb = 0;
	HWND hRasCombo = GetDlgItem(hDlg, IDC_AUTODIAL);

	if (!RasEnumEntries(NULL, NULL, NULL, &cb, NULL) && cb)
	{
		DWORD cEntries;       // Number of Entries found
		LPRASENTRYNAME lpRasEntryName = (LPRASENTRYNAME)LocalAlloc(LPTR, cb);
		if (lpRasEntryName)
		{
			lpRasEntryName[0].dwSize = sizeof(RASENTRYNAME);
			if (!RasEnumEntries(NULL, NULL, lpRasEntryName, &cb, &cEntries) && cEntries)
			{
				for (DWORD i=0; i<cEntries; i++)
				{
					SendMessage(hRasCombo, CB_ADDSTRING, 0, 
									(LPARAM )lpRasEntryName[i].szEntryName);
				}
			}
			LocalFree(lpRasEntryName);
		}

		LPRASCONN lpRasConn = (LPRASCONN) LocalAlloc(LPTR, sizeof(RASCONN));
		if (lpRasConn)
		{
			lpRasConn->dwSize = sizeof(RASCONN);
			DWORD cConnections;
			DWORD nRet = RasEnumConnections(lpRasConn, &lpRasConn->dwSize, &cConnections);

			int i = SendMessage(hRasCombo, CB_FINDSTRINGEXACT,
						(WPARAM )-1, (LPARAM )lpRasConn->szEntryName);
			if (i != CB_ERR)
			{
				SendMessage(hRasCombo, CB_SETCURSEL,
							(WPARAM)i, (LPARAM)0);
			}
			LocalFree(lpRasConn);
		}
	}

	hKey = NULL;
	lRes = RegOpenKeyEx(HKEY_CURRENT_USER, pszAutoDialKey, 0, KEY_QUERY_VALUE, &hKey);
	if (hKey && lRes==ERROR_SUCCESS)
	{
		dwLen = MAXLEN;
		lRes = RegQueryValueEx(hKey, _T("InternetProfile"), 0, &dwType, 
							(BYTE *)szString, &dwLen);
		int nSel = SendMessage(hRasCombo, CB_FINDSTRINGEXACT, (WPARAM )(-1), (LPARAM )(LPCSTR)szString);
		if (nSel != CB_ERR)
		{
			SendMessage(hRasCombo, CB_SETCURSEL, nSel, 0);
		}
		RegCloseKey(hKey);
	}


	//proxy info
	//this structure is necessary because, proxy info is returned in one consecutive buffer
	struct proxytemp
	{
		INTERNET_PROXY_INFO ipi;
		TCHAR				chProxy[MAXLEN];
		TCHAR				chBypass[MAXLEN];
	} proxybuf;

	SendMessage(GetDlgItem(hDlg, IDC_ADDRESS), EM_SETLIMITTEXT, MAXLEN, 0);

	// 5 chars allows 1-99999 for port number
	SendMessage(GetDlgItem(hDlg, IDC_PORT), EM_SETLIMITTEXT, 5, 0);

	proxybuf.ipi.lpszProxy = proxybuf.chProxy;
	proxybuf.ipi.lpszProxyBypass = proxybuf.chBypass;
	ULONG uSize = sizeof(proxybuf);
	if (InternetQueryOption(NULL, INTERNET_OPTION_PROXY , (LPVOID )(&proxybuf), &uSize))
	{
		BOOL bUseProxy = (proxybuf.ipi.dwAccessType == INTERNET_OPEN_TYPE_PROXY);
		SendMessage(GetDlgItem(hDlg, IDC_USEPROXY), BM_SETCHECK, bUseProxy, 0L);
		if (bUseProxy)
		{
			LPTSTR	pszProxy;
			pszProxy = _tcstok((LPTSTR)proxybuf.ipi.lpszProxy, _T(":"));
			SetDlgItemText(hDlg, IDC_ADDRESS, pszProxy);
			pszProxy = _tcstok(NULL, _T(":"));
			SetDlgItemText(hDlg, IDC_PORT, pszProxy);
		}
		else
		{
			EnableWindow(GetDlgItem(hDlg, IDC_ADDRESS), FALSE);
			EnableWindow(GetDlgItem(hDlg, IDC_PORT), FALSE);
		}
	}
	//Use LAN or use Autodial?
	//
	hKey = NULL;
	lRes = RegOpenKeyEx(HKEY_CURRENT_USER, pszUseLanNoAutodial, 0, KEY_QUERY_VALUE, &hKey);
	if (hKey && lRes==ERROR_SUCCESS)
	{
		DWORD dwVal = 0;
		dwLen = sizeof(dwVal);
		lRes = RegQueryValueEx(hKey, _T("EnableAutodial"), 0, &dwType, (BYTE *)&dwVal, &dwLen);
		if (lRes == ERROR_SUCCESS && !dwVal)  {
			//Error checking kinda' blows here, but...
			//
			SendMessage(GetDlgItem(hDlg, IDC_LANNOAUTODIAL), BM_SETCHECK, BST_CHECKED, 0L);
		} else {
			SendMessage(GetDlgItem(hDlg, IDC_LANNOAUTODIAL), BM_SETCHECK, BST_UNCHECKED, 0L);
		}
		RegCloseKey(hKey);
	}
	
}

#endif	//!defined(NOCOMMANDBAR) && !defined(NOOPTIONS)
