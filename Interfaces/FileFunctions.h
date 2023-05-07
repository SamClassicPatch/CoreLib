/* Copyright (c) 2023 Dreamy Cecil
This program is free software; you can redistribute it and/or modify
it under the terms of version 2 of the GNU General Public License as published by
the Free Software Foundation


This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA. */

#ifndef CECIL_INCL_FILEFUNCTIONS_H
#define CECIL_INCL_FILEFUNCTIONS_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

#include <CoreLib/Base/Unzip.h>
#include <CoreLib/Interfaces/DataFunctions.h>

#include <io.h>

typedef CDynamicStackArray<CTFileName> CFileList;

// Interface of useful methods for file manipulation
namespace IFiles {

struct DirToRead {
  CListNode dr_lnNode;
  CTString dr_strDir;
};

enum EFileListFlags {
  FLF_RECURSIVE   = DLI_RECURSIVE, // Look into subdirectories
  FLF_SEARCHCD    = DLI_SEARCHCD,  // List extras from the CD
  FLF_SEARCHMOD   = (1 << 2),      // List extras from the mod
  FLF_ONLYCD      = (1 << 3),      // List exclusively from the CD directory
  FLF_ONLYMOD     = (1 << 4),      // List exclusively from the mod directory
  FLF_IGNORELISTS = (1 << 5),      // Ignore include/exclude lists if playing a mod
};

// Include/exclude lists for base directory writing/reading
CORE_API extern CFileList aBaseWriteInc;
CORE_API extern CFileList aBaseWriteExc;
CORE_API extern CFileList aBaseBrowseInc;
CORE_API extern CFileList aBaseBrowseExc;

// Find any slash in a string and return its position
inline INDEX GetSlashPos(const char *strString) {
  const char *pStart = strString;

  for (; *strString != '\0'; strString++) {
    if (*strString == '\\' || *strString == '/') {
      return (strString - pStart);
    }
  }

  return -1;
};

// CTFileName method from 1.10
inline void SetAbsolutePath(CTFileName &fnm) {
  // Collect path parts
  CTString strRemaining(fnm);
  CStaticStackArray<CTString> astrParts;

  INDEX iSlashPos = GetSlashPos(strRemaining);

  // Invalid path
  if (iSlashPos <= 0) return;

  for (;;) {
    CTString &strBeforeSlash = astrParts.Push();
    CTString strAfterSlash;

    strRemaining.Split(iSlashPos, strBeforeSlash, strAfterSlash);
    strAfterSlash.TrimLeft(strAfterSlash.Length() - 1);
    strRemaining = strAfterSlash;

    iSlashPos = GetSlashPos(strRemaining);

    if (iSlashPos <= 0) {
      astrParts.Push() = strRemaining;
      break;
    }
  }

  INDEX iPart;

  // Remove certain path parts
  for (iPart = 0; iPart < astrParts.Count(); ++iPart) {
    if (astrParts[iPart] != "..") {
      continue;
    }

    // Invalid path
    if (iPart == 0) return;

    // Remove ordered
    CStaticStackArray<CTString> astrShrinked;
    astrShrinked.Push(astrParts.Count() - 2);
    astrShrinked.PopAll();

    for (INDEX iCopiedPart = 0; iCopiedPart < astrParts.Count(); iCopiedPart++)
    {
      if ((iCopiedPart != iPart - 1) && (iCopiedPart != iPart)) {
        astrShrinked.Push() = astrParts[iCopiedPart];
      }
    }

    astrParts.MoveArray(astrShrinked);
    iPart -= 2;
  }

  // Set new content
  strRemaining.Clear();

  for (iPart = 0; iPart < astrParts.Count(); ++iPart) {
    strRemaining += astrParts[iPart];

    if (iPart < astrParts.Count() - 1) {
      #ifdef PLATFORM_WIN32
        strRemaining += "\\";
      #else
        strRemaining += "/";
      #endif
    }
  }

  fnm = strRemaining;
};

// Convert relative paths into absolute paths and add missing backslashes
inline void SetFullDirectory(CTFileName &fnmDir) {
  INDEX iLength = fnmDir.Length();

  // Add missing backslash at the end
  if (fnmDir[iLength - 1] != '\\') {
    fnmDir += CTString("\\");
  }

  // If shorter than 2 characters or doesn't start with a drive directory
  if (iLength < 2 || fnmDir[1] != ':') {
    // Convert relative path into absolute path
    fnmDir = _fnmApplicationPath + fnmDir;
  }

  // Convert the rest of the path into absolute path
  SetAbsolutePath(fnmDir);
};

// Check if the file is readable
inline BOOL IsReadable(const char *strFullPath) {
  FILE *pFile = fopen(strFullPath, "rb");

  if (pFile != NULL) {
    fclose(pFile);
    return TRUE;
  }

  return FALSE;
};

// Load a list of meaningful strings
inline BOOL LoadStringList(CFileList &afnm, const CTFileName &fnmList) {
  try {
    CTFileStream strm;
    strm.Open_t(fnmList);

    // Add every non-empty line with trimmed spaces
    while (!strm.AtEOF()) {
      char strBuffer[1024];
      IData::GetLineFromStream_t(strm, strBuffer, sizeof(strBuffer));

      CTString strLine = strBuffer;
      strLine.TrimSpacesLeft();
      strLine.TrimSpacesRight();

      if (strLine != "") {
        afnm.Push() = strLine;
      }
    }
    return TRUE;

  } catch (char *strError) {
    CPrintF("%s\n", strError);
  }
  return FALSE;
};

// CTFileName comparison method for qsort()
inline int CompareFileNames(const void *pElement1, const void *pElement2)
{
  const CTFileName &fnm1 = **(CTFileName **)pElement1;
  const CTFileName &fnm2 = **(CTFileName **)pElement2;

  return strcmp(fnm1.str_String, fnm2.str_String);
};

// Check if some name pattern matches any file in the list
inline INDEX MatchesList(const CFileList &afnm, const CTString &strPattern)
{
  for (INDEX i = 0; i < afnm.Count(); i++) {
    const CTFileName &fnmCheck = afnm[i];

    // Starts with the pattern or matches the wildcards
    if (strPattern.HasPrefix(fnmCheck) || strPattern.Matches(fnmCheck)) {
      return i;
    }
  }

  return -1;
};

// List files from a specific directory on a disk
inline void ListInDir(const CTFileName &fnmBaseDir, CFileList &afnm,
  const CTString &strDir, const CTString &strPattern, BOOL bRecursive, CFileList *paInclude, CFileList *paExclude)
{
  // Add the directory to search list
  DirToRead *pdrFirst = new DirToRead;
  pdrFirst->dr_strDir = strDir;

  CListHead lhDirs;
  lhDirs.AddTail(pdrFirst->dr_lnNode);

  // While the list of directories is not empty
  while (!lhDirs.IsEmpty())
  {
    // Take the first one
    DirToRead *pdr = LIST_HEAD(lhDirs, DirToRead, dr_lnNode);

    const CTString strCurDir = pdr->dr_strDir;
    delete pdr;

    // Skip if the directory is not allowed
    if (paInclude != NULL && paExclude != NULL
      && (MatchesList(*paInclude, strCurDir) == -1 || MatchesList(*paExclude, strCurDir) != -1)) {
      continue;
    }

    // Start listing the directory
    _finddata_t fdFile;
    long hFile = _findfirst((fnmBaseDir + strCurDir + "*").str_String, &fdFile);

    // Keep going through the files in the directory
    BOOL bFileExists = (hFile != -1);

    while (bFileExists)
    {
      // Skip dummy directories
      if (fdFile.name[0] == '.') {
        bFileExists = (_findnext(hFile, &fdFile) == 0);
        continue;
      }

      // Get path to the file
      CTString strFile = strCurDir + fdFile.name;

      // If it's a directory
      if (fdFile.attrib & _A_SUBDIR) {
        // If reading recursively
        if (bRecursive) {
          // Add this directory to search list
          DirToRead *pdrNew = new DirToRead;
          pdrNew->dr_strDir = strFile + "\\";

          lhDirs.AddTail(pdrNew->dr_lnNode);
        }

      // If it matches the pattern
      } else if (strPattern == "" || strFile.Matches(strPattern)) {
        // Add the file
        CTFileName &fnmPush = afnm.Push();
        fnmPush = strFile;
      }

      // Try to find the next file
      bFileExists = (_findnext(hFile, &fdFile) == 0);
    }
  }
};

// List files from a specific game directory
inline void ListGameFiles(CFileList &afnmFiles, const CTString &strDir, const CTString &strPattern, ULONG ulFlags)
{
  afnmFiles.PopAll();

  const BOOL bRecursive = ulFlags & FLF_RECURSIVE;

  // Don't allow multiple exclusive flags at once
  ASSERTMSG(!(ulFlags & FLF_ONLYCD) == !(ulFlags & FLF_ONLYMOD), "Cannot have multiple exclusive flags in ListGameFiles()!");

  // Make a temporary list
  CFileList afnmTemp;
  BOOL bMod = (_fnmMod != "");
  BOOL bCD = (_fnmCDPath != "");
  BOOL bLists = bMod && !(ulFlags & FLF_IGNORELISTS);

  // List files exclusively from the mod
  if (ulFlags & FLF_ONLYMOD) {
    if (bMod) {
      ListInDir(_fnmApplicationPath + _fnmMod, afnmTemp, strDir, strPattern, bRecursive, NULL, NULL);
    }

  // List files exclusively from the CD
  } else if (ulFlags & FLF_ONLYCD) {
    if (bCD) {
      ListInDir(_fnmCDPath, afnmTemp, strDir, strPattern, bRecursive,
                bLists ? &aBaseBrowseInc : NULL, bLists ? &aBaseBrowseExc : NULL);
    }

  } else {
    // List files from the game directory
    ListInDir(_fnmApplicationPath, afnmTemp, strDir, strPattern, bRecursive,
              bLists ? &aBaseBrowseInc : NULL, bLists ? &aBaseBrowseExc : NULL);

    // List extra files from the CD
    if (ulFlags & FLF_SEARCHCD && bCD) {
      ListInDir(_fnmCDPath, afnmTemp, strDir, strPattern, bRecursive,
                bLists ? &aBaseBrowseInc : NULL, bLists ? &aBaseBrowseExc : NULL);
    }

    // List extra files from the mod directory
    if (ulFlags & FLF_SEARCHMOD && bMod) {
      ListInDir(_fnmApplicationPath + _fnmMod, afnmTemp, strDir, strPattern, bRecursive, NULL, NULL);
    }
  }

  // For each file in the archives
  INDEX ctFilesInZips = IUnzip::GetFileCount();

  for (INDEX iFileInZip = 0; iFileInZip < ctFilesInZips; iFileInZip++) {
    const CTFileName &fnm = IUnzip::GetFileAtIndex(iFileInZip);

    // Skip if not under this directory
    if (bRecursive) {
      if (!fnm.HasPrefix(strDir)) {
        continue;
      }

    } else if (fnm.FileDir() != strDir) {
      continue;
    }

    // Doesn't match the pattern
    if (strPattern != "" && !fnm.Matches(strPattern)) {
      continue;
    }

    // If a mod is active and the file is not in a mod
    if (_fnmMod != "" && !IUnzip::IsFileAtIndexMod(iFileInZip)) {
      // Skip if it doesn't match the browse paths
      if (MatchesList(aBaseBrowseInc, fnm) == -1 || MatchesList(aBaseBrowseExc, fnm) != -1) {
        continue;
      }
    }

    // Add the file
    afnmTemp.Push() = fnm;
  }

  const INDEX ctFiles = afnmTemp.Count();

  // Don't check for duplicates if no files
  if (ctFiles == 0) {
    return;
  }

  // Sort the file list
  qsort(afnmTemp.da_Pointers, afnmTemp.Count(), sizeof(CTFileName *), CompareFileNames);

  // Copy the first file into the final list
  afnmFiles.Push() = afnmTemp[0];

  // Copy the rest of the files if they aren't matching previous files
  for (INDEX iFile = 1; iFile < ctFiles; iFile++)
  {
    if (afnmTemp[iFile] != afnmTemp[iFile - 1]) {
      afnmFiles.Push() = afnmTemp[iFile];
    }
  }
};

}; // namespace

#endif
