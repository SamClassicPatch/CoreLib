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

#include "StdH.h"

#include "ObserverCamera.h"

// Clamp distance from number difference
__forceinline DOUBLE ClampDistDn(DOUBLE dDiff, DOUBLE dDown) {
  DOUBLE d = Abs(dDiff);
  return (d < dDown ? dDown : d);
};

// Clamp distance from vector difference
__forceinline DOUBLE ClampDistDn(FLOAT3D vDiff, DOUBLE dDown) {
  DOUBLE d = vDiff.Length();
  return (d < dDown ? dDown : d);
};

// Calculate position within a Catmull-Rom spline using four points
// Source: https://qroph.github.io/2018/07/30/smooth-paths-using-catmull-rom-splines.html
template<class Type> __forceinline
Type CatmullRom(const Type pt0, const Type pt1, const Type pt2, const Type pt3, DOUBLE dR, DOUBLE dTension)
{
  // [Cecil] FIXME: Other values make movement less smooth between points, possibly due
  // to the fact that it's using a small segment at a time instead of the entire path.
  static const DOUBLE fAlpha = 0.0;
  DOUBLE t01 = pow(ClampDistDn(pt0 - pt1, 0.01), fAlpha);
  DOUBLE t12 = pow(ClampDistDn(pt1 - pt2, 0.01), fAlpha);
  DOUBLE t23 = pow(ClampDistDn(pt2 - pt3, 0.01), fAlpha);

  Type m1 = (pt2 - pt1 + ((pt1 - pt0) / t01 - (pt2 - pt0) / (t01 + t12)) * t12) * (1.0 - dTension);
  Type m2 = (pt2 - pt1 + ((pt3 - pt2) / t23 - (pt3 - pt1) / (t12 + t23)) * t12) * (1.0 - dTension);

  Type a = (pt1 - pt2) * +2.0 + m1 + m2;
  Type b = (pt1 - pt2) * -3.0 - m1 - m1 - m2;
  Type c = m1;
  Type d = pt1;

  return a * dR * dR * dR + b * dR * dR + c * dR + d;
};

void CObserverCamera::ReadPos(CameraPos &cp) {
  // Playback config exhausted
  if (cam_strmScript.AtEOF()) {
    // Buffer last position so it plays until that one as well
    if (!cam_acpCurve[3].bLast) {
      cam_acpCurve[3].bLast = TRUE;

    // Otherwise reset playback
    } else {
      Reset(TRUE);
    }
    return;
  }

  try {
    CTString strLine;
    cam_strmScript.GetLine_t(strLine);

    strLine.ScanF("%g: %g: %g %g %g:%g %g %g:%g",
      &cp.tmTick, &cp.fSpeed, &cp.vPos(1), &cp.vPos(2), &cp.vPos(3),
      &cp.aRot(1), &cp.aRot(2), &cp.aRot(3), &cp.fFOV);

  } catch (char *strError) {
    CPrintF(TRANS("OCAM Error: %s\n"), strError);
  }
};

void CObserverCamera::WritePos(CameraPos &cp) {
  try {
    CTString strLine(0, "%g: %g: %g %g %g:%g %g %g:%g",
      _pTimer->GetLerpedCurrentTick() - cam_tmStartTime, 1.0f,
      cp.vPos(1), cp.vPos(2), cp.vPos(3), cp.aRot(1), cp.aRot(2), cp.aRot(3), cp.fFOV);

    cam_strmScript.PutLine_t(strLine);

  } catch (char *strError) {
    CPrintF(TRANS("OCAM Error: %s\n"), strError);
  }
};

static inline void SetSpeed(FLOAT fSpeed) {
  static CSymbolPtr pfRealTimeFactor("dem_fRealTimeFactor");

  if (!pfRealTimeFactor.Exists()) {
    ASSERTALWAYS("'dem_fRealTimeFactor' symbol doesn't exist!");
    return;
  }

  pfRealTimeFactor.GetFloat() = fSpeed;
};

// Initialize camera interface
void CObserverCamera::Init(void)
{
  _pShell->DeclareSymbol("user INDEX ocam_bActive;", &cam_bActive);

  _pShell->DeclareSymbol("user INDEX ocam_bMoveForward;",      &cam_ctl.bMoveF);
  _pShell->DeclareSymbol("user INDEX ocam_bMoveBackward;",     &cam_ctl.bMoveB);
  _pShell->DeclareSymbol("user INDEX ocam_bMoveLeft;",         &cam_ctl.bMoveL);
  _pShell->DeclareSymbol("user INDEX ocam_bMoveRight;",        &cam_ctl.bMoveR);
  _pShell->DeclareSymbol("user INDEX ocam_bMoveUp;",           &cam_ctl.bMoveU);
  _pShell->DeclareSymbol("user INDEX ocam_bMoveDown;",         &cam_ctl.bMoveD);
  _pShell->DeclareSymbol("user INDEX ocam_bTurnBankingLeft;",  &cam_ctl.bBankingL);
  _pShell->DeclareSymbol("user INDEX ocam_bTurnBankingRight;", &cam_ctl.bBankingR);
  _pShell->DeclareSymbol("user INDEX ocam_bZoomIn;",           &cam_ctl.bZoomIn);
  _pShell->DeclareSymbol("user INDEX ocam_bZoomOut;",          &cam_ctl.bZoomOut);
  _pShell->DeclareSymbol("user INDEX ocam_bZoomDefault;",      &cam_ctl.bZoomDefault);
  _pShell->DeclareSymbol("user INDEX ocam_bResetToPlayer;",    &cam_ctl.bResetToPlayer);
  _pShell->DeclareSymbol("user INDEX ocam_bFollowPlayer;",     &cam_ctl.bFollowPlayer);
  _pShell->DeclareSymbol("user INDEX ocam_bSnapshot;",         &cam_ctl.bSnapshot);

  _pShell->DeclareSymbol("persistent user INDEX ocam_bSmoothPlayback;", &cam_bSmoothPlayback);
  _pShell->DeclareSymbol("persistent user FLOAT ocam_fSmoothTension;", &cam_fSmoothTension);
  _pShell->DeclareSymbol("persistent user FLOAT ocam_fSpeed;", &cam_fSpeed);
  _pShell->DeclareSymbol("persistent user FLOAT ocam_fSmoothMovement;", &cam_fSmoothMovement);
  _pShell->DeclareSymbol("persistent user FLOAT ocam_fSmoothRotation;", &cam_fSmoothRotation);
  _pShell->DeclareSymbol("persistent user FLOAT ocam_fFollowDist;", &cam_fFollowDist);
};

// Start camera for a game (or a currently playing demo)
void CObserverCamera::Start(const CTFileName &fnmDemo) {
  // Reset variables
  Reset();

  if (fnmDemo == "") return;

  // Simple text file with a suffix
  cam_fnmDemo = fnmDemo.NoExt() + "_CAM.txt";

  // Open existing file for playback
  try {
    cam_strmScript.Open_t(cam_fnmDemo);

  } catch (char *strError) {
    (void)strError;
    return;
  }

  cam_bPlayback = TRUE;

  // Read the first positions immediately
  ReadPos(cam_acpCurve[1]);
  ReadPos(cam_acpCurve[2]);
  ReadPos(cam_acpCurve[3]);
  cam_acpCurve[0] = cam_acpCurve[1]; // Skip point 0 since the curve goes from 1 to 2

  SetSpeed(cam_acpCurve[0].fSpeed);
};

// Stop playback or camera altogether
void CObserverCamera::Reset(BOOL bPlayback) {
  // Full reset
  if (!bPlayback) {
    cam_ctl.Reset();

    cam_fnmDemo = CTString("");
    cam_tmStartTime = -100.0f;
    cam_tvDelta.tv_llValue = 0;
  }

  cam_bPlayback = FALSE;
  cam_strmScript.Close();

  for (INDEX i = 0; i < 4; i++) {
    cam_acpCurve[i] = CameraPos();
  }
  cam_cpCurrent = CameraPos();

  cam_vMovement = FLOAT3D(0, 0, 0);
  cam_aRotation = ANGLE3D(0, 0, 0);

  SetSpeed(1.0f);
};

// Check if camera is on
BOOL CObserverCamera::IsActive(void) {
  // Camera can only be used during observing or in demos
  const BOOL bObserver = (GetGameAPI()->GetCurrentSplitCfg() == CGame::SSC_OBSERVER);

  if (bObserver || _pNetwork->IsPlayingDemo()) {
    return cam_bActive || cam_bPlayback;
  }

  return FALSE;
};

// Start recording into a file
BOOL CObserverCamera::StartRecording(void) {
  // Already recording
  if (cam_strmScript.GetDescription() != "") return TRUE;

  try {
    cam_strmScript.Create_t(cam_fnmDemo);
    return TRUE;

  } catch (char *strError) {
    CPrintF(TRANS("OCAM Error: %s\n"), strError);
  }

  return FALSE;
};

// Free fly camera movement during the game
CObserverCamera::CameraPos &CObserverCamera::FreeFly(CPlayerEntity *penObserving) {
  CameraPos &cp = cam_cpCurrent;

  // "Per second" speed multiplier for this frame
  static const DOUBLE dTicks = (1.0 / _pTimer->TickQuantum);
  const DOUBLE dTimeMul = dTicks * cam_tvDelta.GetSeconds();

  // Camera rotation
  {
    cam_fSmoothRotation = Clamp(cam_fSmoothRotation, 0.0f, 1.0f);
    const BOOL bInstantRotation = (cam_fSmoothRotation == 1.0f);
    const BOOL bFollowing = (cam_ctl.bFollowPlayer && penObserving != NULL);

    // Input rotation
    ANGLE3D aRotate;

    // Focus on the current player
    if (bFollowing) {
      // Direction towards the player
      CPlacement3D plView = IWorld::GetViewpoint(penObserving, TRUE);
      const FLOAT3D vDir = (plView.pl_PositionVector - cp.vPos).SafeNormalize();

      // Relative angle towards the player
      DirectionVectorToAnglesNoSnap(vDir, aRotate);
      aRotate -= cp.aRot;

      aRotate(1) = NormalizeAngle(aRotate(1));
      aRotate(2) = NormalizeAngle(aRotate(2));

    // Manual mouse input
    } else {
      aRotate(1) = _pInput->GetAxisValue(MOUSE_X_AXIS) * -0.5f;
      aRotate(2) = _pInput->GetAxisValue(MOUSE_Y_AXIS) * +0.5f;
    }

    aRotate(3) = (cam_ctl.bBankingL - cam_ctl.bBankingR) * 0.5f;

    // Set immediately
    if (bInstantRotation) {
      cam_aRotation = aRotate;
      cam_aRotation(3) *= dTicks;
      cam_ctl.bBankingL = cam_ctl.bBankingR = 0;

    // Smooth rotation
    } else {
      // Use cosine in order to make real values slightly lower than they are (e.g. 0.5 -> ~0.3)
      const FLOAT fSpeedMul = 1.0f - Cos(cam_fSmoothRotation * 90);
      cam_aRotation += (aRotate - cam_aRotation) * dTimeMul * fSpeedMul;

      // Override directional rotation while following
      if (bFollowing) {
        cam_aRotation(1) = aRotate(1) * dTimeMul * cam_fSmoothRotation;
        cam_aRotation(2) = aRotate(2) * dTimeMul * cam_fSmoothRotation;
      }
    }

    cp.aRot += cam_aRotation;

    // Snap banking angle on sharp movement
    if (bInstantRotation && Abs(cam_aRotation(3)) > 0.0f) {
      Snap(cp.aRot(3), 10.0f);
    }
  }

  // Camera movement
  if (cam_fSpeed != 0.0f) {
    // Movement vector
    FLOAT3D vMoveDir(0, 0, 0);

    // Follow the player and always stay close enough
    if (cam_fFollowDist >= 0.0f && penObserving != NULL) {
      CPlacement3D plView = IWorld::GetViewpoint(penObserving, TRUE);
      FLOAT3D vToPlayer = (plView.pl_PositionVector - cp.vPos);

      if (vToPlayer.Length() > cam_fFollowDist) {
        vMoveDir = vToPlayer.SafeNormalize() * cam_fSpeed;
      }
    }

    // Add input vector to the any current movement
    FLOAT3D vInputDir(cam_ctl.bMoveR - cam_ctl.bMoveL, cam_ctl.bMoveU - cam_ctl.bMoveD, cam_ctl.bMoveB - cam_ctl.bMoveF);
    const FLOAT fInputLength = vInputDir.Length();

    if (fInputLength > 0.01f) {
      FLOATmatrix3D mRot;
      MakeRotationMatrixFast(mRot, ANGLE3D(cp.aRot(1), 0, 0)); // Only heading direction

      // Normalize vector, apply current rotation and speed
      vMoveDir += (vInputDir / fInputLength) * mRot * cam_fSpeed;
    }

    // Set immediately
    if (cam_fSmoothMovement >= 1.0f) {
      cam_vMovement = vMoveDir;

    // Smooth movement
    } else {
      // Use cosine in order to make real values slightly lower than they are (e.g. 0.5 -> ~0.3)
      const FLOAT fSpeedMul = 1.0f - Cos(ClampDn(cam_fSmoothMovement, 0.0f) * 90);
      cam_vMovement += (vMoveDir - cam_vMovement) * dTimeMul * fSpeedMul;
    }

    cp.vPos += cam_vMovement * dTimeMul;
  }

  if (cam_ctl.bZoomDefault) {
    cp.fFOV = 90.0f;
  } else {
    cp.fFOV = Clamp(FLOAT(cp.fFOV + (cam_ctl.bZoomOut - cam_ctl.bZoomIn) * dTimeMul), 10.0f, 170.0f);
  }

  // Snap back to view of the current player
  if (cam_ctl.bResetToPlayer && penObserving != NULL)
  {
    CPlacement3D plView = IWorld::GetViewpoint(penObserving, TRUE);
    cp.vPos = plView.pl_PositionVector;
    cp.aRot = plView.pl_OrientationAngle;
  }

  // Take position snapshot for a demo
  if (cam_ctl.bSnapshot && cam_fnmDemo != "") {
    cam_ctl.bSnapshot = FALSE;

    // Try starting the recording first
    if (StartRecording()) {
      CTString strTime;
      IData::PrintDetailedTime(strTime, DOUBLE(_pTimer->GetLerpedCurrentTick() - cam_tmStartTime));

      CPrintF(TRANS("OCAM: Took snapshot at %s\n"), strTime.str_String);
      WritePos(cp);
    }
  }

  return cp;
};

// Update the camera and render the world through it
BOOL CObserverCamera::Update(CEntity *pen, CDrawPort *pdp) {
  // Time variables
  static CTimerValue tvLastTick;
  const CTimerValue tvRealTick = _pTimer->GetHighPrecisionTimer();

  // Get start time when it's time to render
  if (cam_tmStartTime == -100.0f) {
    cam_tmStartTime = _pTimer->GetLerpedCurrentTick();
    cam_tvDelta = DOUBLE(0);

  // Calculate delta from last tick
  } else {
    cam_tvDelta = (tvRealTick - tvLastTick);
  }

  // Save this tick
  tvLastTick = tvRealTick;

  // Camera is currently disabled
  if (!IsActive()) {
    cam_vMovement = FLOAT3D(0, 0, 0);
    cam_aRotation = ANGLE3D(0, 0, 0);
    return FALSE;
  }

  // Determine camera position for this frame
  CameraPos cp;

  // Camera playback
  if (cam_bPlayback && !cam_bActive) {
    // Read next position when the destination expires
    TIME tmNow = _pTimer->GetLerpedCurrentTick() - cam_tmStartTime;

    // Skip around
    while (cam_bPlayback && tmNow > cam_acpCurve[2].tmTick) {
      // Advance positions
      INDEX i;
      for (i = 0; i < 3; i++) {
        cam_acpCurve[i] = cam_acpCurve[i + 1];
      }

      // Read the next one
      ReadPos(cam_acpCurve[i]);

      SetSpeed(cam_acpCurve[1].fSpeed);
    }

    // Interpolate between two positions
    const CameraPos *acp = cam_acpCurve;
    FLOAT fRatio = (tmNow - acp[1].tmTick) / (acp[2].tmTick - acp[1].tmTick);

    // Move through a curve between two points
    if (cam_bSmoothPlayback) {
      cam_fSmoothTension = Clamp(cam_fSmoothTension, 0.0f, 1.0f);
      cp.vPos = CatmullRom(acp[0].vPos, acp[1].vPos, acp[2].vPos, acp[3].vPos, fRatio, cam_fSmoothTension);
      cp.aRot = CatmullRom(acp[0].aRot, acp[1].aRot, acp[2].aRot, acp[3].aRot, fRatio, cam_fSmoothTension);
      cp.fFOV = CatmullRom(acp[0].fFOV, acp[1].fFOV, acp[2].fFOV, acp[3].fFOV, fRatio, cam_fSmoothTension);

    // Linear movement from one point to another
    } else {
      cp.vPos = Lerp(acp[1].vPos, acp[2].vPos, fRatio);
      cp.aRot = Lerp(acp[1].aRot, acp[2].aRot, fRatio);
      cp.fFOV = Lerp(acp[1].fFOV, acp[2].fFOV, fRatio);
    }

  // Free fly movement
  } else {
    CPlayerEntity *penObserving = NULL;

    if (IsDerivedFromID(pen, CPlayerEntity_ClassID)) {
      penObserving = (CPlayerEntity *)pen;
    }

    cp = FreeFly(penObserving);
  }

  // Prepare view projection
  CPerspectiveProjection3D prProjection;
  prProjection.FOVL() = cp.fFOV;
  prProjection.ScreenBBoxL() = FLOATaabbox2D(FLOAT2D(0, 0), FLOAT2D(pdp->GetWidth(), pdp->GetHeight()));
  prProjection.AspectRatioL() = 1.0f;
  prProjection.FrontClipDistanceL() = 0.3f;

  // Render view from the camera
  CAnyProjection3D apr;
  apr = prProjection;
  apr->ViewerPlacementL() = cp.GetPlacement();

  CWorld &wo = _pNetwork->ga_World;
  RenderView(wo, *(CEntity *)NULL, apr, *pdp);

  // Listen to world sounds
  cam_sliWorld.sli_vPosition = cp.vPos;
  MakeRotationMatrixFast(cam_sliWorld.sli_mRotation, cp.aRot);
  cam_sliWorld.sli_fVolume = 1.0f;
  cam_sliWorld.sli_vSpeed = cam_vMovement;
  cam_sliWorld.sli_penEntity = NULL;
  cam_sliWorld.sli_fFilter = 0.0f;

  CEnvironmentType &et = wo.wo_aetEnvironmentTypes[0];
  cam_sliWorld.sli_iEnvironmentType = et.et_iType;
  cam_sliWorld.sli_fEnvironmentSize = et.et_fSize;

  _pSound->Listen(cam_sliWorld);
  return TRUE;
};
