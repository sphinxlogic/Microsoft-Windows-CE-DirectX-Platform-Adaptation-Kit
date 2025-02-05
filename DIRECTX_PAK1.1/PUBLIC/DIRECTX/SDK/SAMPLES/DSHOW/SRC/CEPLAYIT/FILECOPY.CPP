//==========================================================================;
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
//  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
//  PURPOSE.
//
//  Copyright (c) 1999-2000  Microsoft Corporation.  All Rights Reserved.
//
//--------------------------------------------------------------------------;
//
//   Helper code for copying a file from the PC into the local file system
//   and later deleting it.
//

//
// Function prototypes
//
#include <windows.h>
#include "ceshio.h"

//
// Constants
//
#define BUFFER_SIZE   32768

//
// Global data
//
BOOL bCreatedFile = FALSE;
TCHAR szTemporaryFile[MAX_PATH];      // Global filename for temporary copy

//
// Function prototypes
//
BOOL ReadFromPC(TCHAR *szFile, LPTSTR lpCmdLine);
BOOL DeleteTemporaryFile(TCHAR *szFile);
void TRACE(LPCTSTR szFormat, ...);
BOOL CopyFilePPSH(LPCTSTR szSrc, LPCTSTR szDst);
void ShowMemoryInformation(void);




BOOL ReadFromPC(TCHAR *szFile, LPTSTR lpCmdLine)
{
    // Convert "cesh" to "ppsh" to conform to expected string
    lstrcpy(szFile, TEXT("ppsh:"));
    lstrcat(szFile, lpCmdLine+5);

    // Create the destination filename (without "PPSH:")
    lstrcpy(szTemporaryFile, TEXT("\\windows\\"));
    lstrcat(szTemporaryFile, lpCmdLine+5);

    // Use common code from PPSHCOPY to copy the file to the local file system
    if (! CopyFilePPSH(szFile, szTemporaryFile))
        return FALSE;

    // Update filename with local filename for use by RenderFile()
    lstrcpy(szFile, szTemporaryFile);

    // Set global flag for later deleting this file
    bCreatedFile = TRUE;
    return TRUE;
}


BOOL DeleteTemporaryFile(void)
{
    BOOL bRC = TRUE;

    // If we created a temporary file to store a media file that was read
    // through the CESH connection, delete it now.
    if (bCreatedFile)
    {
        bRC = DeleteFile(szTemporaryFile);
        if (!bRC)
        {
            TRACE(TEXT("Error 0x%x deleting local file!\r\n"), GetLastError);
        }
    }

    return bRC;
}


void TRACE(LPCTSTR szFormat, ...) {

   TCHAR szBuffer[512];  // Large buffer for very long filenames (like with HTTP)

   // Build the message.
   _tcscpy(szBuffer, TEXT("CEPlayit: "));
   va_list pArgs;
   va_start(pArgs, szFormat);
   wvsprintf(szBuffer + 10, szFormat, pArgs);
   va_end(pArgs);

   // Send the message string to the debugger as well.
   _tcscat(szBuffer, TEXT("\r\n"));
   OutputDebugString(szBuffer);
}


//
//
//  Courtesy of PPSHCOPY tool
//
//
BOOL CopyFilePPSH(LPCTSTR szSrc, LPCTSTR szDst) {

   BYTE  *pbBuffer = NULL;
   int    fileSrc = -1, fileDst = -1, length;
   HANDLE hFileSrc = INVALID_HANDLE_VALUE, hFileDst = INVALID_HANDLE_VALUE;
   DWORD  dwBytesLeft = 0, dwBytesBlock = 0, dwBytesTransferred = 0, dwFileSize = 0;
   BOOL   fSuccess = FALSE, fCopyError = FALSE;
   TCHAR  szFullDst[MAX_PATH + 6], *szTemp;

   // Make sure we were passed valid args.
   if (!szSrc || !*szSrc) {
      SetLastError(ERROR_INVALID_PARAMETER);
      TRACE(TEXT("CopyFilePPSH: Invalid parameters!"));
      return FALSE;
   }

   // Check to see if the destination is actually a path (ends in '\'
   // or only contains "ppsh:").
   length = _tcslen(szDst);
   if (((length >  0) && (*(szDst + length - 1) == TEXT('\\'))) ||
       ((length == 5) && !_tcsnicmp(szDst, TEXT("ppsh:"), 5)))
   {
      // Build a new destination path - start by copying the path to our buffer.
      _tcscpy(szFullDst, szDst);

      // Locate the just the file portion of the source file path.
      szDst = szSrc;
      if (szTemp = _tcsrchr(szDst, TEXT('\\'))) {
         szDst = szTemp + 1;
      }
      if (szTemp = _tcsrchr(szDst, TEXT(':'))) {
         szDst = szTemp + 1;
      }

      // Append the source file name to the destination file path.
      _tcscat(szFullDst, szDst);

      // Point our destination file name to our new full path buffer.
      szDst = szFullDst;
   }

   // Tell the user what we are about to do.
   TRACE(TEXT("Copying \"%s\" to \"%s\"..."), szSrc, szDst);

   __try {

      if (!(pbBuffer = new BYTE[BUFFER_SIZE])) {
         TRACE(TEXT("Not enough free memory to allocate a %u byte copy buffer."), BUFFER_SIZE);
         __leave;
      }

      // Check to see if the source file is across PPSH.
      if (!_tcsnicmp(szSrc, TEXT("ppsh:"), 5)) {

         // Open the source file for read across PPSH.
         if ((fileSrc = U_ropen(szSrc + 5, _O_RDONLY | _O_SEQUENTIAL)) == -1) {
            TRACE(TEXT("Error %u trying to open file \"%s\"."), GetLastError(), szSrc);
            __leave;
         }

         // Get the size of the file
         dwBytesLeft = U_rlseek(fileSrc, 0, SEEK_END);
         U_rlseek(fileSrc, 0, SEEK_SET);

      } else {

         // Open the source file for read on the device.
         hFileSrc = CreateFile(szSrc, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
         if (hFileSrc == INVALID_HANDLE_VALUE) {
            TRACE(TEXT("Error %u trying to open file \"%s\"."), GetLastError(), szSrc);
            __leave;
         }

         // Get the size of the file.
         dwBytesLeft = GetFileSize(hFileSrc, NULL);
      }

      // Make sure we have a valid file size.
      if (dwBytesLeft == 0xFFFFFFFF) {
         TRACE(TEXT("Error %u trying to get size of file \"%s\"."), GetLastError(), szSrc);
         __leave;
      }

      // Save file size in case of error
      dwFileSize = dwBytesLeft;
      TRACE(TEXT("File size is %d bytes."), dwFileSize);

      // Check to see if the destination file is across PPSH.
      if (!_tcsnicmp(szDst, TEXT("ppsh:"), 5)) {

         // Try _O_CREAT | _O_EXCL | _O_WRONLY | _O_SEQUENTIAL first and display overwrite warning?

         // Open the destination file for write across PPSH.
         if ((fileDst = U_ropen(szDst + 5, _O_CREAT | _O_WRONLY | _O_SEQUENTIAL)) == -1) {
            TRACE(TEXT("Error %u trying to create \"%s\"."), GetLastError(), szDst);
            __leave;
         }

      } else {

         // Try CREATE_NEW first and display overwrite warning?

         // Open the destination file for write on the device.
         hFileDst = CreateFile(szDst, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS,
                               FILE_ATTRIBUTE_NORMAL, NULL);
         if (hFileDst == INVALID_HANDLE_VALUE) {
            TRACE(TEXT("Error %u trying to create \"%s\"."), GetLastError(), szDst);
            __leave;
         }
      }

      // Loop through the file to read block / write block
      while (dwBytesLeft > 0) {

         // Compute the size of this block of data we are about to transfer.
         dwBytesBlock = (dwBytesLeft < BUFFER_SIZE) ? dwBytesLeft : BUFFER_SIZE;

         // Check to see if the source file is across PPSH.
         if (hFileSrc == INVALID_HANDLE_VALUE) {

            // Read a block of data from the source PPSH file.
            if (U_rread(fileSrc, pbBuffer, dwBytesBlock) != (int)dwBytesBlock) {
               TRACE(TEXT("Error %u reading from \"%s\"."), GetLastError(), szSrc);
               fCopyError = TRUE;
               __leave;
            }

         } else {

            // Read a block of data from the source device file.
            if (!ReadFile(hFileSrc, pbBuffer, dwBytesBlock, &(dwBytesTransferred = 0), NULL) ||
                (dwBytesBlock != dwBytesTransferred))
            {
               TRACE(TEXT("Error %u reading from \"%s\"."), GetLastError(), szSrc);
               __leave;
            }
         }

         // Check to see if the destination file is across PPSH.
         if (hFileDst == INVALID_HANDLE_VALUE) {

            // Write a block of data to the destination PPSH file.
            if (U_rwrite(fileDst, pbBuffer, dwBytesBlock) != (int)dwBytesBlock) {
               TRACE(TEXT("Error %u writing to \"%s\"."), GetLastError(), szDst);
               __leave;
            }

         } else {

            // Write a block of data to the destination device file.
            if (!WriteFile(hFileDst, pbBuffer, dwBytesBlock, &(dwBytesTransferred = 0), NULL) ||
                (dwBytesBlock != dwBytesTransferred))
            {
               int nError = GetLastError();
               if (nError == ERROR_DISK_FULL) {
                  TRACE(TEXT("There is not enough free storage space on your device to write the file."));
                  fCopyError = TRUE;
                  ShowMemoryInformation();
               }
               else
                  TRACE(TEXT("Error %u writing to \"%s\"."), nError, szDst);

               __leave;
            }
         }

         // Reduce our "bytes left" counter by the number of bytes we just transferred.
         dwBytesLeft -= dwBytesBlock;
      }

      // If we made it this far, things look good.
      fSuccess = TRUE;

      TRACE(TEXT("Successfully copied \"%s\" to \"%s\"."), szSrc, szDst);

   // Global cleanup code
   } __finally {

      // Free our copy buffer.
      if (pbBuffer) {
         delete[] pbBuffer;
      }

      // Close any source PPSH file we may have opened.
      if (fileSrc != -1) {
         U_rclose(fileSrc);
      }

      // Close any destination PPSH file we may have opened.
      if (fileDst != -1) {
         U_rclose(fileDst);
      }

      // Close any source device file we may have opened.
      if (hFileSrc != INVALID_HANDLE_VALUE) {
         CloseHandle(hFileSrc);
      }

      // Close any destination device file we may have opened.
      if (hFileDst != INVALID_HANDLE_VALUE) {
         CloseHandle(hFileDst);
      }

      // If the file was read or written incorrectly, delete it, since this function
      // will return failure.
      if (fCopyError) {
        if (! DeleteFile(szDst))
            TRACE(TEXT("Error %u deleting file \"%s\"."), GetLastError(), szDst);
      }
   }

   return fSuccess;
}



void ShowMemoryInformation(void)
{
    MEMORYSTATUS mst;
    memset(&mst, 0, sizeof(mst));
    mst.dwLength = sizeof(mst);

    GlobalMemoryStatus(&mst);

    DWORD kbPercent = mst.dwMemoryLoad;
    DWORD kbTotal = mst.dwTotalPhys / 1024;
    DWORD kbUsed = (mst.dwTotalPhys - mst.dwAvailPhys) / 1024;
    DWORD kbFree = mst.dwAvailPhys / 1024;
    TRACE(TEXT("Memory (KB):      %7d total   %7d used   %7d free"), kbTotal, kbUsed, kbFree);

    STORE_INFORMATION sti;
    GetStoreInformation(&sti);

    DWORD stikbTotal = sti.dwStoreSize / 1024;
    DWORD stikbUsed = (sti.dwStoreSize - sti.dwFreeSize) / 1024;
    DWORD stikbFree = sti.dwFreeSize / 1024;
    TRACE(TEXT("File Store (KB):  %7d total   %7d used   %7d free"), stikbTotal, stikbUsed, stikbFree);

    DWORD cpStore, cpRam, cbPage;
    GetSystemMemoryDivision(&cpStore, &cpRam, &cbPage);
    TRACE(TEXT("System page size: %d bytes"), cbPage);
    TRACE(TEXT("RAM pages: %d (%d bytes)     Store pages: %d (%d bytes)"),
          cpRam, (cpRam * cbPage), cpStore, (cpStore * cbPage));
}



