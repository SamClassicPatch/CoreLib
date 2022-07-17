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

// Get scaling multiplier based on screen height
#define HEIGHT_SCALING(DrawPort) ((FLOAT)DrawPort->GetHeight() / 480.0f)

// Interface of methods for rendering
class IRender {
  public:
    // Calculate horizontal FOV according to the aspect ratio
    // Can be cancelled using one of the following:
    //   AdjustVFOV( FLOAT2D( 4, 3 ), fHFOV )
    //   AdjustHFOV( FLOAT2D( 1, 1 ), fHFOV )
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
    // Can be cancelled using one of the following:
    //   AdjustVFOV( FLOAT2D( vScreen(2), vScreen(1) ), fHFOV )
    //   AdjustHFOV( FLOAT2D( vScreen(1), vScreen(2) * 0.75f ), fHFOV )
    static inline void AdjustVFOV(const FLOAT2D &vScreen, FLOAT &fHFOV) {
      // Get inverted aspect ratio of the current resolution
      FLOAT fInverseAspectRatio = vScreen(2) / vScreen(1);

      // Take current FOV angle and apply aspect ratio to it
      FLOAT fVerticalAngle = Tan(fHFOV * 0.5f) * fInverseAspectRatio;

      // 90 FOV on 4:3 or 106.26 FOV on 16:9 will become 73.74...
      fHFOV = 2.0f * ATan(fVerticalAngle);
    };

    // Begin special model rendering
    static inline void BeginModelRendering(CAnyProjection3D &apr, CDrawPort *pdp, BOOL bAdjustFOV)
    {
      // Set to adjust model FOV, if needed
      GetAPI()->SetAdjustFOV(bAdjustFOV ? 1 : 0);

      // Begin model rendering
      BeginModelRenderingView(apr, pdp);
    };

    // End special model rendering
    static inline void EndModelRendering(BOOL bRestoreOrtho = TRUE)
    {
      // Reset to FOV adjustment
      GetAPI()->SetAdjustFOV(-1);

      // End model rendering
      EndModelRenderingView(bRestoreOrtho);
    };
};

#endif
