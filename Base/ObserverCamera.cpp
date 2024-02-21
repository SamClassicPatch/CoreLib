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

void CObserverCamera::ReadPos(CameraPos &cp) {
  // Playback config exhausted
  if (cam_strmScript.AtEOF()) {
    Reset(TRUE);
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
  _pShell->DeclareSymbol("user INDEX ocam_bSnapshot;",         &cam_ctl.bSnapshot);

  _pShell->DeclareSymbol("persistent user FLOAT ocam_fSpeed;", &cam_fSpeed);
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
  ReadPos(cam_acpCurve[0]);
  ReadPos(cam_acpCurve[1]);

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

  for (INDEX i = 0; i < 2; i++) {
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
    // Input rotation
    ANGLE3D aRotate;

    // Manual mouse input
    aRotate(1) = _pInput->GetAxisValue(MOUSE_X_AXIS) * -0.5f;
    aRotate(2) = _pInput->GetAxisValue(MOUSE_Y_AXIS) * +0.5f;
    aRotate(3) = (cam_ctl.bBankingL - cam_ctl.bBankingR) * 0.5f;

    // Set immediately
    cam_aRotation = aRotate;
    cam_aRotation(3) *= dTicks;
    cam_ctl.bBankingL = cam_ctl.bBankingR = 0;

    cp.aRot += cam_aRotation;

    // Snap banking angle on sharp movement
    if (Abs(cam_aRotation(3)) > 0.0f) {
      Snap(cp.aRot(3), 10.0f);
    }
  }

  // Camera movement
  if (cam_fSpeed != 0.0f) {
    // Movement vector
    FLOAT3D vMoveDir(cam_ctl.bMoveR - cam_ctl.bMoveL, cam_ctl.bMoveU - cam_ctl.bMoveD, cam_ctl.bMoveB - cam_ctl.bMoveF);
    const FLOAT fMoveLength = vMoveDir.Length();

    if (fMoveLength > 0.01f) {
      FLOATmatrix3D mRot;
      MakeRotationMatrixFast(mRot, ANGLE3D(cp.aRot(1), 0, 0)); // Only heading direction

      // Normalize vector, apply current rotation and speed
      vMoveDir = (vMoveDir / fMoveLength) * mRot * cam_fSpeed;
    }

    // Set immediately
    cam_vMovement = vMoveDir;

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
    while (cam_bPlayback && tmNow > cam_acpCurve[1].tmTick) {
      // Advance positions
      cam_acpCurve[0] = cam_acpCurve[1];

      // Read the next one
      ReadPos(cam_acpCurve[1]);

      SetSpeed(cam_acpCurve[1].fSpeed);
    }

    // Interpolate between two positions
    const CameraPos &cpSrc = cam_acpCurve[1];
    const CameraPos &cpDst = cam_acpCurve[0];
    FLOAT fRatio = (tmNow - cpSrc.tmTick) / (cpDst.tmTick - cpSrc.tmTick);

    cp.vPos = Lerp(cpSrc.vPos, cpDst.vPos, fRatio);
    cp.aRot = Lerp(cpSrc.aRot, cpDst.aRot, fRatio);
    cp.fFOV = Lerp(cpSrc.fFOV, cpDst.fFOV, fRatio);

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
