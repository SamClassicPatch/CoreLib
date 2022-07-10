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

#ifndef CECIL_INCL_RENDERFUNCTIONS_H
#define CECIL_INCL_RENDERFUNCTIONS_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

// Interface of methods for rendering
class IRender {
  public:
    // Main render space, if available
    static CDrawPort *pdpRenderSpace;
    
  public:
    // Get main render space
    static inline CDrawPort *GetDrawPort(void) {
      ASSERT(pdpRenderSpace != NULL);
      return pdpRenderSpace;
    };

    // Set main render space
    static inline void SetDrawPort(CDrawPort *pdp) {
      pdpRenderSpace = pdp;
    };

    // Get render space sizes
    static inline FLOAT2D GetScreenSize(void) {
      return FLOAT2D(GetDrawPort()->GetWidth(), GetDrawPort()->GetHeight());
    };

    // Calculate horizontal FOV according to the aspect ratio
    static inline void AdjustHFOV(const FLOAT2D &vScreen, FLOAT &fHFOV) {
      // Get aspect ratio of the current resolution
      FLOAT fAspectRatio = vScreen(1) / vScreen(2);

      // 4:3 resolution = 1.0 ratio; 16:9 = 1.333 etc.
      FLOAT fSquareRatio = fAspectRatio * (3.0f / 4.0f);

      // Take current FOV angle and apply square ratio to it
      FLOAT fVerticalAngle = Tan(fHFOV * 0.5f) * fSquareRatio;

      // 90 FOV on 16:9 resolution will become 106.26...
      fHFOV = 2.0f * ATan(fVerticalAngle);
    };

    // Calculate vertical FOV from horizontal FOV according to the aspect ratio
    static inline void AdjustVFOV(const FLOAT2D &vScreen, FLOAT &fHFOV) {
      // Get inverted aspect ratio of the current resolution
      FLOAT fInverseAspectRatio = vScreen(2) / vScreen(1);

      // Take current FOV angle and apply aspect ratio to it
      FLOAT fVerticalAngle = Tan(fHFOV * 0.5f) * fInverseAspectRatio;

      // 90 FOV on 4:3 or 106.26 FOV on 16:9 will become 73.74...
      fHFOV = 2.0f * ATan(fVerticalAngle);
    };
};

#endif
