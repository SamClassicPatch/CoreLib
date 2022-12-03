/* Copyright (c) 2022 Dreamy Cecil
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

#ifndef CECIL_INCL_DATAFUNCTIONS_H
#define CECIL_INCL_DATAFUNCTIONS_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

#include <Engine/Base/Unzip.h>

#include <io.h>

typedef CDynamicStackArray<CTFileName> CDynamicFileStack;

// Interface of useful methods for data manipulation
class IData {
  public:
    struct DirToRead {
      CListNode dr_lnNode;
      CTString dr_strDir;
    };

    enum EFileListFlags {
      FLF_RECURSIVE = DLI_RECURSIVE, // Look into subdirectories
      FLF_SEARCHCD  = DLI_SEARCHCD,  // List extras from the disk
      FLF_SEARCHMOD = (1 << 2),      // List extras from the mod
      FLF_MODONLY   = (1 << 3),      // List exclusively from the mod directory
    };

  public:
    // Convert forward slashes to backslashes in a file path
    static inline void ConvertSlashes(char *str) {
      while (*str != '\0') {
        if (*str == '/') {
          *str = '\\';
        }
        ++str;
      }
    };

    // CTFileName comparison method for qsort()
    static inline int CompareFileNames(const void *pElement1, const void *pElement2)
    {
      const CTFileName &fnm1 = **(CTFileName **)pElement1;
      const CTFileName &fnm2 = **(CTFileName **)pElement2;

      return strcmp(fnm1.str_String, fnm2.str_String);
    };

    // Check if some name pattern matches any file in the list
    static inline INDEX FileMatchesList(const CDynamicFileStack &afnm, const CTString &strPattern)
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
    static inline void ListFilesInDir(const CTFileName &fnmBaseDir, CDynamicFileStack &afnm,
      const CTString &strDir, const CTString &strPattern, BOOL bRecursive)
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
    static inline void ListGameFiles(CDynamicFileStack &afnmFiles, const CTString &strDir, const CTString &strPattern, ULONG ulFlags)
    {
      afnmFiles.PopAll();

      const BOOL bRecursive = ulFlags & FLF_RECURSIVE;

      // Make a temporary list
      CDynamicFileStack afnmTemp;

      // List files exclusively from the mod
      if (ulFlags & FLF_MODONLY && _fnmMod != "") {
        ListFilesInDir(_fnmApplicationPath + _fnmMod, afnmTemp, strDir, strPattern, bRecursive);

      } else {
        // List files from the game directory
        ListFilesInDir(_fnmApplicationPath, afnmTemp, strDir, strPattern, bRecursive);

        // List extra files from the game disk
        if (ulFlags & FLF_SEARCHCD) {
          ListFilesInDir(_fnmCDPath, afnmTemp, strDir, strPattern, bRecursive);
        }

        // List extra files from the mod directory
        if (ulFlags & FLF_SEARCHMOD && _fnmMod != "") {
          ListFilesInDir(_fnmApplicationPath + _fnmMod, afnmTemp, strDir, strPattern, bRecursive);
        }
      }

      // For each file in the archives
      INDEX ctFilesInZips = UNZIPGetFileCount();

      for (INDEX iFileInZip = 0; iFileInZip < ctFilesInZips; iFileInZip++) {
        const CTFileName &fnm = UNZIPGetFileAtIndex(iFileInZip);

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
};

#endif