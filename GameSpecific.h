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

#ifndef CECIL_INCL_GAMESPECIFIC_H
#define CECIL_INCL_GAMESPECIFIC_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

// Choose value based on configuration
#if SE1_GAME == SS_TFE
  #define CHOOSE_FOR_GAME(_TFE105, _TSE105, _TSE107) _TFE105
#elif SE1_VER == SE1_105
  #define CHOOSE_FOR_GAME(_TFE105, _TSE105, _TSE107) _TSE105
#else
  #define CHOOSE_FOR_GAME(_TFE105, _TSE105, _TSE107) _TSE107
#endif

// Addresses of specific engine elements
#ifdef NDEBUG
  // CRenderer::Render()
  #define ADDR_RENDERER_RENDER (CHOOSE_FOR_GAME(0x601A8CD0, 0x60178DB0, 0x601B4A00))

  // &_areRenderers[0]
  #define ADDR_RENDERER_ARRAY ((CRenderer *)(ULONG *)CHOOSE_FOR_GAME(0x6029C4F8, 0x6026C538, 0x602CDAF0))

  // Static variables from Unzip.cpp
  #define ADDR_UNZIP_CRITSEC  ((CTCriticalSection *)(ULONG *)CHOOSE_FOR_GAME(0x602A0388, 0x602703C8, 0x602D31C8)) // &zip_csLock
  #define ADDR_UNZIP_HANDLES  ((ULONG *)CHOOSE_FOR_GAME(0x60285030, 0x60255070, 0x602B6298)) // &_azhHandles
  #define ADDR_UNZIP_ENTRIES  ((ULONG *)CHOOSE_FOR_GAME(0x60285040, 0x60255080, 0x602B62A8)) // &_azeFiles
  #define ADDR_UNZIP_ARCHIVES ((ULONG *)CHOOSE_FOR_GAME(0x60285024, 0x60255064, 0x602B628C)) // &_afnmArchives

// Debug addresses
#elif SE1_VER == SE1_107
  // CRenderer::Render()
  #define ADDR_RENDERER_RENDER (0x10008062)

  // &_areRenderers[0]
  #define ADDR_RENDERER_ARRAY ((CRenderer *)(ULONG *)0x10440458)

  // Static variables from Unzip.cpp
  #define ADDR_UNZIP_CRITSEC  ((CTCriticalSection *)(ULONG *)0x10446BC8) // &zip_csLock
  #define ADDR_UNZIP_HANDLES  ((ULONG *)0x10424158) // &_azhHandles
  #define ADDR_UNZIP_ENTRIES  ((ULONG *)0x10424168) // &_azeFiles
  #define ADDR_UNZIP_ARCHIVES ((ULONG *)0x1042414C) // &_afnmArchives
#endif

#endif
