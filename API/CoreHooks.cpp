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

#include "StdH.h"

#include "Networking/Modules/ClientLogging.h"

// Auto update shadows upon loading into worlds
INDEX gam_bAutoUpdateShadows = TRUE;

// Queue shadow updating for the next connection to a server
static BOOL _bQueueShadowUpdate = TRUE;

// Fix broken shadows and lights by updating them
void UpdateShadows(void)
{
  FOREACHINDYNAMICCONTAINER(IWorld::GetWorld()->wo_cenEntities, CEntity, iten) {
    // CLight_ClassID
    if (!IsDerivedFromID(&*iten, 200)) continue;

    // Update shadow layers for each light
    CLightSource *pls = iten->GetLightSource();

    if (pls != NULL) {
      pls->FindShadowLayers(FALSE);
    }
  }

  // Update shadows from the sun and such
  IWorld::GetWorld()->CalculateDirectionalShadows();
};

// Called every simulation tick
void ICoreHooks::OnTick(void)
{
  // Call step function for each plugin
  FOREACHPLUGINHANDLER(GetPluginAPI()->cProcessors, IProcessingEvents, pEvents) {
    if ((IAbstractEvents *)pEvents == NULL) continue;

    pEvents->OnStep();
  }
};

// Called every render frame
void ICoreHooks::OnFrame(CDrawPort *pdp)
{
  // Interact with Steam
  GetSteamAPI()->Update();

  // Update input values
  for (INDEX iAxis = 0; iAxis < MAX_OVERALL_AXES; iAxis++) {
    inp_afAxisValues[iAxis] = _pInput->GetAxisValue(iAxis);
  }

  // Call frame function for each plugin
  FOREACHPLUGINHANDLER(GetPluginAPI()->cProcessors, IProcessingEvents, pEvents) {
    if ((IAbstractEvents *)pEvents == NULL) continue;

    pEvents->OnFrame(pdp);
  }
};

// Called before redrawing game view
void ICoreHooks::OnPreDraw(CDrawPort *pdp)
{
  // Call pre-draw function for each plugin
  FOREACHPLUGINHANDLER(GetPluginAPI()->cRenderers, IRenderingEvents, pEvents) {
    if ((IAbstractEvents *)pEvents == NULL) continue;

    pEvents->OnPreDraw(pdp);
  }
};

// Called after redrawing game view
void ICoreHooks::OnPostDraw(CDrawPort *pdp)
{
  // Call post-draw function for each plugin
  FOREACHPLUGINHANDLER(GetPluginAPI()->cRenderers, IRenderingEvents, pEvents) {
    if ((IAbstractEvents *)pEvents == NULL) continue;

    pEvents->OnPostDraw(pdp);
  }
};

// Called after rendering the world
void ICoreHooks::OnRenderView(CWorld &wo, CEntity *penViewer, CAnyProjection3D &apr, CDrawPort *pdp)
{
  // Call render view function for each plugin
  FOREACHPLUGINHANDLER(GetPluginAPI()->cRenderers, IRenderingEvents, pEvents) {
    if ((IAbstractEvents *)pEvents == NULL) continue;

    pEvents->OnRenderView(wo, penViewer, apr, pdp);
  }
};

// Called after starting world simulation
void ICoreHooks::OnGameStart(void)
{
  // Call game start function for each plugin
  FOREACHPLUGINHANDLER(GetPluginAPI()->cGameEvents, IGameEvents, pEvents) {
    if ((IAbstractEvents *)pEvents == NULL) continue;

    pEvents->OnGameStart();
  }
};

// Called after changing the level
void ICoreHooks::OnChangeLevel(void)
{
  // Call level change function for each plugin
  FOREACHPLUGINHANDLER(GetPluginAPI()->cGameEvents, IGameEvents, pEvents) {
    if ((IAbstractEvents *)pEvents == NULL) continue;

    pEvents->OnChangeLevel();
  }
};

// Called before stopping world simulation
void ICoreHooks::OnGameStop(void)
{
  // Reset CAM for the next start
  GetGameAPI()->GetCamera().Reset();

  // Call game stop function for each plugin
  FOREACHPLUGINHANDLER(GetPluginAPI()->cGameEvents, IGameEvents, pEvents) {
    if ((IAbstractEvents *)pEvents == NULL) continue;

    pEvents->OnGameStop();
  }

  // Queue shadow updating again
  _bQueueShadowUpdate = TRUE;

  // Reset all clients
  CActiveClient::ResetAll();

  // Save client log by the end of the game
  IClientLogging::SaveLog();
};

// Called after saving the game
void ICoreHooks::OnGameSave(const CTFileName &fnmSave)
{
  // Call game save function for each plugin
  FOREACHPLUGINHANDLER(GetPluginAPI()->cGameEvents, IGameEvents, pEvents) {
    if ((IAbstractEvents *)pEvents == NULL) continue;

    pEvents->OnGameSave(fnmSave);
  }
};

// Called after loading a saved game
void ICoreHooks::OnGameLoad(const CTFileName &fnmSave)
{
  // Call game load function for each plugin
  FOREACHPLUGINHANDLER(GetPluginAPI()->cGameEvents, IGameEvents, pEvents) {
    if ((IAbstractEvents *)pEvents == NULL) continue;

    pEvents->OnGameLoad(fnmSave);
  }

  // Update shadow maps
  if (gam_bAutoUpdateShadows) {
    UpdateShadows();
  }
};

// Called every time a new player is added
void ICoreHooks::OnAddPlayer(CPlayerTarget &plt, BOOL bLocal)
{
  // Call player addition function for each plugin
  FOREACHPLUGINHANDLER(GetPluginAPI()->cNetworkEvents, INetworkEvents, pEvents) {
    if ((IAbstractEvents *)pEvents == NULL) continue;

    pEvents->OnAddPlayer(plt, bLocal);
  }

  // Logic for connecting players
  if (bLocal && !_pNetwork->IsServer())
  {
    // Update shadow maps only if queued
    if (_bQueueShadowUpdate) {
      _bQueueShadowUpdate = FALSE;

      if (gam_bAutoUpdateShadows) {
        UpdateShadows();
      }
    }
  }
};

// Called every time a player is removed
void ICoreHooks::OnRemovePlayer(CPlayerTarget &plt, BOOL bLocal)
{
  // Call player removal function for each plugin
  FOREACHPLUGINHANDLER(GetPluginAPI()->cNetworkEvents, INetworkEvents, pEvents) {
    if ((IAbstractEvents *)pEvents == NULL) continue;

    pEvents->OnRemovePlayer(plt, bLocal);
  }
};

// Called after starting demo playback
void ICoreHooks::OnDemoPlay(const CTFileName &fnmDemo)
{
  // Start CAM for the demo
  GetGameAPI()->GetCamera().Start(fnmDemo);

  // Call demo play function for each plugin
  FOREACHPLUGINHANDLER(GetPluginAPI()->cDemoEvents, IDemoEvents, pEvents) {
    if ((IAbstractEvents *)pEvents == NULL) continue;

    pEvents->OnDemoPlay(fnmDemo);
  }

  // Update shadow maps
  if (gam_bAutoUpdateShadows) {
    UpdateShadows();
  }
};

// Called after starting demo recording
void ICoreHooks::OnDemoStart(const CTFileName &fnmDemo)
{
  // Call demo start function for each plugin
  FOREACHPLUGINHANDLER(GetPluginAPI()->cDemoEvents, IDemoEvents, pEvents) {
    if ((IAbstractEvents *)pEvents == NULL) continue;

    pEvents->OnDemoStart(fnmDemo);
  }
};

// Called after stopping demo recording
void ICoreHooks::OnDemoStop(void)
{
  // Call demo stop function for each plugin
  FOREACHPLUGINHANDLER(GetPluginAPI()->cDemoEvents, IDemoEvents, pEvents) {
    if ((IAbstractEvents *)pEvents == NULL) continue;

    pEvents->OnDemoStop();
  }
};

// Called after finishing reading the world file
void ICoreHooks::OnWorldLoad(CWorld *pwo, const CTFileName &fnmWorld)
{
  // Call world load function for each plugin
  FOREACHPLUGINHANDLER(GetPluginAPI()->cWorldEvents, IWorldEvents, pEvents) {
    if ((IAbstractEvents *)pEvents == NULL) continue;

    pEvents->OnWorldLoad(pwo, fnmWorld);
  }
};
