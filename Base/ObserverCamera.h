/* Copyright (c) 2024 Dreamy Cecil
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

#ifndef CECIL_INCL_OBSERVERCAMERA_H
#define CECIL_INCL_OBSERVERCAMERA_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

// Interface for freecam observing during a game
class CORE_API CObserverCamera {
  public:
    // Camera control
    struct CameraControl {
      INDEX bMoveF, bMoveB, bMoveL, bMoveR, bMoveU, bMoveD;
      INDEX bBankingL, bBankingR, bZoomIn, bZoomOut;
      INDEX bZoomDefault, bResetToPlayer, bFollowPlayer, bSnapshot;

      CameraControl() {
        Reset();
      };

      // Reset all controls
      void Reset(void) {
        bMoveF = bMoveB = bMoveL = bMoveR = bMoveU = bMoveD = FALSE;
        bBankingL = bBankingR = bZoomIn = bZoomOut = FALSE;
        bZoomDefault = bResetToPlayer = bFollowPlayer = bSnapshot = FALSE;
      };
    };

    // Camera position
    struct CameraPos {
      TIME tmTick;
      FLOAT fSpeed;
      FLOAT3D vPos;
      ANGLE3D aRot;
      FLOAT fFOV;

      CameraPos() : vPos(0, 0, 0), aRot(0, 0, 0), fFOV(90), fSpeed(1), tmTick(0)
      {
      };

      inline CPlacement3D GetPlacement(void) const {
        return CPlacement3D(vPos, aRot);
      };
    };

  public:
    // Camera control (fields outside cam_ctl aren't reset between camera activations)
    CameraControl cam_ctl;
    BOOL cam_bActive; // Dynamic camera toggle
    FLOAT cam_fSpeed; // Movement speed multiplier
    FLOAT cam_fSmoothMovement; // Factor for smooth camera movement
    FLOAT cam_fSmoothRotation; // Factor for smooth camera rotation

    BOOL cam_bPlayback; // Currently playing back the recording
    CTFileName cam_fnmDemo; // Currently playing demo
    CTFileStream cam_strmScript;

    TIME cam_tmStartTime; // Simulation time when camera started
    CTimerValue cam_tvDelta; // Time since last render frame

    CameraPos cam_acpCurve[2]; // Camera positions for a curve (playback mode)
    CameraPos cam_cpCurrent; // Current camera position (freecam mode)

    // Absolute movement & rotation speed
    FLOAT3D cam_vMovement;
    ANGLE3D cam_aRotation;

    CSoundListener cam_sliWorld; // Listener for world sounds

  public:
    // Constructor
    CObserverCamera() : cam_bActive(FALSE) {
      cam_fSpeed = 1.0f;
      cam_fSmoothMovement = 1.0f;
      cam_fSmoothRotation = 1.0f;
      Reset();
    };

    // Initialize camera interface
    void Init(void);

    void ReadPos(CameraPos &cp);
    void WritePos(CameraPos &cp);

    // Start camera for a game (or a currently playing demo)
    void Start(const CTFileName &fnmDemo);

    // Stop playback or camera altogether
    virtual void Reset(BOOL bPlayback = FALSE);

    // Check if camera is on
    virtual BOOL IsActive(void);

    // Start recording into a file
    virtual BOOL StartRecording(void);

    // Free fly camera movement during the game
    CameraPos &FreeFly(CPlayerEntity *penObserving);

    // Update the camera and render the world through it
    BOOL Update(CEntity *pen, CDrawPort *pdp);
};

#endif
