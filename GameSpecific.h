/* Copyright (c) 2022-2023 Dreamy Cecil
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

#ifndef CECIL_INCL_GAMESPECIFIC_H
#define CECIL_INCL_GAMESPECIFIC_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

// Choose value based on configuration
#if SE1_VER == SE1_105 && SE1_GAME == SS_TFE
  #define CHOOSE_FOR_GAME(_TFE105, _TSE105, _TSE107) _TFE105
#elif SE1_VER == SE1_105 && SE1_GAME == SS_TSE
  #define CHOOSE_FOR_GAME(_TFE105, _TSE105, _TSE107) _TSE105
#elif SE1_VER == SE1_107
  #define CHOOSE_FOR_GAME(_TFE105, _TSE105, _TSE107) _TSE107
#else
  #error Unsupported engine version!
#endif

// Addresses of specific engine elements
#ifdef NDEBUG
  // InitStreams()
  #define ADDR_INITSTREAMS (CHOOSE_FOR_GAME(0x600DA2C0, 0x600AA2C0, 0x600E65F0))

  // CRenderer::Render()
  #define ADDR_RENDERER_RENDER (CHOOSE_FOR_GAME(0x601A8CD0, 0x60178DB0, 0x601B4A00))

  // &_areRenderers[0]
  #define ADDR_RENDERER_ARRAY ((CRenderer *)(ULONG *)CHOOSE_FOR_GAME(0x6029C4F8, 0x6026C538, 0x602CDAF0))

  // Static variables from Unzip.cpp
  #define ADDR_UNZIP_CRITSEC  ((CTCriticalSection *)(ULONG *)CHOOSE_FOR_GAME(0x602A0388, 0x602703C8, 0x602D31C8)) // &zip_csLock
  #define ADDR_UNZIP_HANDLES  ((ULONG *)CHOOSE_FOR_GAME(0x60285030, 0x60255070, 0x602B6298)) // &_azhHandles
  #define ADDR_UNZIP_ENTRIES  ((ULONG *)CHOOSE_FOR_GAME(0x60285040, 0x60255080, 0x602B62A8)) // &_azeFiles
  #define ADDR_UNZIP_ARCHIVES ((ULONG *)CHOOSE_FOR_GAME(0x60285020, 0x60255060, 0x602B6288)) // &_afnmArchives

  // UNZIP* methods
  #define ADDR_UNZIP_OPEN      (CHOOSE_FOR_GAME(0x600E19E0, 0x600B1A70, 0x600EDD50)) // UNZIPOpen_t()
  #define ADDR_UNZIP_GETSIZE   (CHOOSE_FOR_GAME(0x600E1D90, 0x600B1E20, 0x600EE100)) // UNZIPGetSize()
  #define ADDR_UNZIP_READBLOCK (CHOOSE_FOR_GAME(0x600E1DF0, 0x600B1E80, 0x600EE160)) // UNZIPReadBlock_t()
  #define ADDR_UNZIP_CLOSE     (CHOOSE_FOR_GAME(0x600E20F0, 0x600B2180, 0x600EE460)) // UNZIPClose()
  #define ADDR_UNZIP_GETFILECOUNT     (CHOOSE_FOR_GAME(0x600E18C0, 0x600B1950, 0x600EDC30)) // UNZIPGetFileCount()
  #define ADDR_UNZIP_GETFILEATINDEX   (CHOOSE_FOR_GAME(0x600E18D0, 0x600B1960, 0x600EDC40)) // UNZIPGetFileAtIndex()
  #define ADDR_UNZIP_ISFILEATINDEXMOD (CHOOSE_FOR_GAME(0x600E18F0, 0x600B1980, 0x600EDC60)) // UNZIPIsFileAtIndexMod()

// Debug addresses
#elif SE1_VER == SE1_107
  // InitStreams()
  #define ADDR_INITSTREAMS (0x10007B49)

  // CRenderer::Render()
  #define ADDR_RENDERER_RENDER (0x10008062)

  // &_areRenderers[0]
  #define ADDR_RENDERER_ARRAY ((CRenderer *)(ULONG *)0x10440458)

  // Static variables from Unzip.cpp
  #define ADDR_UNZIP_CRITSEC  ((CTCriticalSection *)(ULONG *)0x10446BC8) // &zip_csLock
  #define ADDR_UNZIP_HANDLES  ((ULONG *)0x10424158) // &_azhHandles
  #define ADDR_UNZIP_ENTRIES  ((ULONG *)0x10424168) // &_azeFiles
  #define ADDR_UNZIP_ARCHIVES ((ULONG *)0x10424148) // &_afnmArchives

  // UNZIP* methods
  #define ADDR_UNZIP_OPEN      (0x10002C20) // UNZIPOpen_t()
  #define ADDR_UNZIP_GETSIZE   (0x1000720C) // UNZIPGetSize()
  #define ADDR_UNZIP_READBLOCK (0x10008693) // UNZIPReadBlock_t()
  #define ADDR_UNZIP_CLOSE     (0x100088FA) // UNZIPClose()
  #define ADDR_UNZIP_GETFILECOUNT     (0x10001456) // UNZIPGetFileCount()
  #define ADDR_UNZIP_GETFILEATINDEX   (0x10005268) // UNZIPGetFileAtIndex()
  #define ADDR_UNZIP_ISFILEATINDEXMOD (0x1000A45C) // UNZIPIsFileAtIndexMod()
#endif

#endif
