/* Copyright (c) 2023-2024 Dreamy Cecil
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

#ifndef CECIL_INCL_COMMONCORE_H
#define CECIL_INCL_COMMONCORE_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

// Don't warn about identifier truncation
#pragma warning(disable: 4786)

// Main engine components
#include <Engine/Engine.h>
#include <Engine/CurrentVersion.h>

// Dummy variables for Revolution support
#if SE1_GAME == SS_REV
  static CTFileName _fnmCDPath = CTString("");
  static CTFileName _fnmMod = CTString("");
#endif

// Extras
#include <Extras/XGizmo/Base/IniConfig.h>
#include <Extras/XGizmo/Interfaces/Data.h>
#include <Extras/XGizmo/Interfaces/Directories.h>
#include <Extras/XGizmo/Interfaces/Files.h>
#include <Extras/XGizmo/Interfaces/Libraries.h>
#include <Extras/XGizmo/Interfaces/Properties.h>
#include <Extras/XGizmo/Interfaces/Render.h>
#include <Extras/XGizmo/Interfaces/World.h>
#include <Extras/XGizmo/Objects/MapStructure.h>
#include <Extras/XGizmo/Objects/StructPtr.h>
#include <Extras/XGizmo/Objects/SymbolPtr.h>

// Classics Patch configuration
#include <CoreLib/Config.h>

// Compatibility features
#include <CoreLib/GameSpecific.h>

#if SE1_VER != SE1_110
  // Declare shell function arguments
  #define SHELL_FUNC_ARGS void *__pFirstArg

  // Begin shell function code
  #define BEGIN_SHELL_FUNC void *pArgs = (void *)&__pFirstArg

#else
  // Declare shell function arguments
  #define SHELL_FUNC_ARGS void *pArgs

  // Begin shell function code
  #define BEGIN_SHELL_FUNC NOTHING
#endif

// Next argument in the symbol function call
#define NEXT_ARG(Type) (*((Type *&)pArgs)++)

// Translate a string that has already been translated in vanilla localizations
#define LOCALIZE(ConstString) ((char *)TranslateConst(ConstString, 0))

#endif
