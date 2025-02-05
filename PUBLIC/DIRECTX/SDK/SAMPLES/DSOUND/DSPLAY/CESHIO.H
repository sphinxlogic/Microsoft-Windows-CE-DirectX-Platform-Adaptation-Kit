//==========================================================================;
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
//  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
//  PURPOSE.
//
//  Copyright (c) 1992 - 1999  Microsoft Corporation.  All Rights Reserved.
//
//--------------------------------------------------------------------------;
//


#define _O_RDONLY	0x0000	/* open for reading only */
#define _O_WRONLY	0x0001	/* open for writing only */
#define _O_RDWR 	0x0002	/* open for reading and writing */
#define _O_APPEND	0x0008	/* writes done at eof */

#define _O_CREAT	0x0100	/* create and open file */
#define _O_TRUNC	0x0200	/* open and truncate */
#define _O_EXCL 	0x0400	/* open only if file doesn't already exist */

int     pp_open(WCHAR *pFileName, UINT uiMode);
int     pp_read(int h, BYTE * pBuffer, int iCount);
int     pp_write(int h, BYTE * pBuffer, int iCount);
int     pp_lseek(int h, int iOffset, int iMode);
int     pp_close(int h);
