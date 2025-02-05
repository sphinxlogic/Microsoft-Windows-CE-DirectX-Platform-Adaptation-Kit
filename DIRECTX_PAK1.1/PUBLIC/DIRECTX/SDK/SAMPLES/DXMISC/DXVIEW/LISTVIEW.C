/****************************************************************************

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.

    Copyright (C) 1995-2000 Microsoft Corporation. All Rights Reserved.

    PROGRAM: listview.c

    PURPOSE: ListView helper code for DX View application

****************************************************************************/

#include <stdio.h>
#include "dxview.h"

//
// Constants
//
#define LV_COLUMN_WIDTH     38
#define LV_VALUE_WIDTH      20

//
// Global data
//
HWND        g_hwndLV;        // List view


//================================================================
// AddMoreCapsToLV is like AddCapsToLV, except it doesn't add the
// column headers like AddCapsToLV does.
//================================================================
void AddMoreCapsToLV(CAPDEF *pcd, LPVOID pv)
{
    DWORD       dwValue;
    D3DVALUE    dvValue;
    TCHAR       szBuff[64];

    while(pcd->szName && *pcd->szName)
    {
        dwValue = *(DWORD *)(((BYTE *)pv) + pcd->dwOffset);

        switch (pcd->dwFlag)
        {
            case 0:
                LVAddText(g_hwndLV, 0, TEXT("%s"), pcd->szName);
                Int2Str(szBuff,dwValue);
                LVAddText(g_hwndLV, 1, TEXT("%s"), szBuff);
                break;
            case 0xFFFFFFFF:    // Hex
                LVAddText(g_hwndLV, 0, TEXT("%s"), pcd->szName);
                LVAddText(g_hwndLV, 1, TEXT("0x%08X"), dwValue);
                break;
            case 0xBFFFFFFF:    // FLOAT Support for new DX6 "D3DVALUE" values in D3DDeviceDesc
                LVAddText(g_hwndLV, 0, TEXT("%s"), pcd->szName);
                dvValue = *(D3DVALUE *)(((BYTE *)pv) + pcd->dwOffset);
                LVAddText(g_hwndLV, 1, TEXT("%G"), dvValue);
                break;
            case 0x7FFFFFFF:    // HEX Support for new DX6 "WORD" values in D3DDeviceDesc
                LVAddText(g_hwndLV, 0, TEXT("%s"), pcd->szName);
                dwValue = *(WORD *)(((BYTE *)pv) + pcd->dwOffset);
                LVAddText(g_hwndLV, 1, TEXT("0x%04X"), dwValue);
                break;
            case 0x3FFFFFFF:    // VAL Support for new DX6 "WORD" values in D3DDeviceDesc
                LVAddText(g_hwndLV, 0, TEXT("%s"), pcd->szName);
                dwValue = *(WORD *)(((BYTE *)pv) + pcd->dwOffset);
                Int2Str(szBuff,dwValue);
                LVAddText(g_hwndLV, 1, TEXT("%s"), szBuff);
                break;
            default:
                if (pcd->dwFlag & dwValue)
                {
                    LVAddText(g_hwndLV, 0, pcd->szName);
                    LVAddText(g_hwndLV, 1, c_szYes);
                }
                else if (g_dwViewState == IDM_VIEWALL)
                {
                    LVAddText(g_hwndLV, 0, pcd->szName);
                    LVAddText(g_hwndLV, 1, c_szNo);
                }
                break;
        }
        pcd++;  // Get next Cap bit definition
    }
}


//================================================================
// AddColsToLV adds the column headers but no data.
//================================================================
void AddColsToLV(void)
{
    LVAddColumn(g_hwndLV, 0, TEXT("Name"), LV_COLUMN_WIDTH);
    LVAddColumn(g_hwndLV, 1, TEXT("Value"), LV_VALUE_WIDTH);
}

/*----------------------------------------------------------------------------*\
\*----------------------------------------------------------------------------*/
void AddCapsToLV(CAPDEF *pcd, LPVOID pv)
{
    AddColsToLV();
    AddMoreCapsToLV(pcd, pv);
}

/*----------------------------------------------------------------------------*\
\*----------------------------------------------------------------------------*/
void LVAddColumn(HWND hwndLV, int i, TCHAR *name, int width)
{
    LV_COLUMN col;

    if (i == 0)
    {
        while(ListView_DeleteColumn(hwndLV, 0))
            ;
    }

    col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
    col.fmt  = LVCFMT_LEFT;
    col.pszText = name;
    col.cchTextMax = 0;
    col.cx = width * g_tmAveCharWidth;
    col.iSubItem = 0;
    ListView_InsertColumn(hwndLV, i, &col);
}

/*----------------------------------------------------------------------------*\
\*----------------------------------------------------------------------------*/
int LVAddText(HWND hwndLV, int col, const TCHAR *sz, ...)
{
    LV_ITEM lvi;
    TCHAR    ach[80];
    va_list vl;

    va_start(vl, sz );

    wvsprintf(ach,sz, vl);

    lvi.mask        = LVIF_TEXT;
    lvi.iSubItem    = 0;
    lvi.state       = 0;
    lvi.stateMask   = 0;
    lvi.pszText     = ach;
    lvi.cchTextMax  = 0;
    lvi.iImage      = 0;
    lvi.lParam      = 0;

    if (col == 0)
    {
        lvi.iItem    = 0x7FFF;
        lvi.iSubItem = 0;
        return ListView_InsertItem(hwndLV, &lvi);
    }
    else
    {
        lvi.iItem    = ListView_GetItemCount(hwndLV)-1;
        lvi.iSubItem = col;
        return ListView_SetItem(hwndLV, &lvi);
    }
    va_end(vl);
}

/*----------------------------------------------------------------------------*\
\*----------------------------------------------------------------------------*/
void LVDeleteAllItems( HWND hwndLV )
{

    //Used by DPLay asynchronous enum sessions
    if (g_idTimer)
    {

        BOOL    bDataAvailable;
        LPLV_INSTANCEGUIDSTRUCT lpInstGuidStruct = NULL;
        LV_ITEM lvi;
        int     i, count;

        KillTimer(g_hwndMain, g_idTimer );
        g_idTimer = 0;

        memset( &lvi, 0, sizeof( LV_ITEM ));

        count = ListView_GetItemCount( g_hwndLV );

        for ( i = 0; i < count; i++ )
        {
            lvi.iItem = i;
            lvi.mask = LVIF_PARAM;
            lvi.lParam = 0;

            bDataAvailable =ListView_GetItem( g_hwndLV, &lvi );

            if ((bDataAvailable) && (lvi.lParam))
                LocalFree( (LPVOID) lvi.lParam );

        }
    }

    ListView_DeleteAllItems( hwndLV );
}

/*----------------------------------------------------------------------------*\
\*----------------------------------------------------------------------------*/
void ResetListViewRefresh( HWND hwndLV )
{
    BOOL bDataAvailable;
    int i       = 0,
        count   = ListView_GetItemCount( hwndLV );

    LPLV_INSTANCEGUIDSTRUCT lpInstGuidStruct = NULL;
    LV_ITEM lvi;

    memset( &lvi, 0, sizeof(LV_ITEM) );

    while( i < count)
    {
        lvi.iItem = i;
        lvi.mask = LVIF_PARAM;
        lvi.lParam = 0;

        bDataAvailable = ListView_GetItem( hwndLV, &lvi );

        if (bDataAvailable)
        {
            lpInstGuidStruct = (LPLV_INSTANCEGUIDSTRUCT) lvi.lParam;

            if (lpInstGuidStruct)
            {
                //Mark this session to be deleted next time if it does not
                //respond to this enum request.
                lpInstGuidStruct->dwRefresh = FALSE;
            }
        }
        else
        {
            //This item has no data, so it must be invalid.
            ListView_DeleteItem( hwndLV, i);
        }

        i++;
    }

}


