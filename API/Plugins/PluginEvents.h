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

#ifndef CECIL_INCL_PLUGINEVENTS_H
#define CECIL_INCL_PLUGINEVENTS_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

// Abstract class for plugin events
class CORE_API IAbstractEvents {
  public:
    // Index of the handler in the container
    INDEX _iHandler;

    // Default constructor
    IAbstractEvents() : _iHandler(-1)
    {
    };

    // Destructor with automatic unregistering
    virtual ~IAbstractEvents() {
      Unregister();
    };

  public:
    // Container to utilize for registering and unregistering
    virtual CPluginInterfaces *GetContainer(void) {
      return NULL;
    };

    // Register events
    void Register(void) {
      CPluginInterfaces *pCont = GetContainer();

      if (pCont == NULL) {
        ASSERTALWAYS("Pointer to the container of event handlers has not been set!");
        return;
      }

      // Restore the pointer if it has a dedicated slot
      if (_iHandler != -1) {
        pCont->sa_Array[_iHandler] = this;

      // Add to handlers if it's not there
      } else if (!pCont->IsMember(this)) {
        _iHandler = pCont->Count();
        pCont->Add(this);

      // Weird edge case with the pointer in the container but with an invalid index
      } else {
        ASSERT(FALSE);
        _iHandler = pCont->GetIndex(this);
      }
    };

    // Unregister events
    void Unregister(void) {
      CPluginInterfaces *pCont = GetContainer();
      if (pCont == NULL) return;

      // Remove the pointer from the dedicated slot
      ASSERT(_iHandler != -1);
      pCont->sa_Array[_iHandler] = NULL;
    };
};

// Main plugin events
class IProcessingEvents : public IAbstractEvents {
  public:
    // Return handlers container
    virtual CPluginInterfaces *GetContainer(void) {
      return &GetPluginAPI()->cProcessors;
    };

    virtual void OnStep(void); // Every simulation tick for synchronized logic
    virtual void OnFrame(CDrawPort *pdp); // After rendering everything
};

// Rendering events
class IRenderingEvents : public IAbstractEvents {
  public:
    // Return handlers container
    virtual CPluginInterfaces *GetContainer(void) {
      return &GetPluginAPI()->cRenderers;
    };

    virtual void OnPreDraw(CDrawPort *pdp); // Before drawing the game view
    virtual void OnPostDraw(CDrawPort *pdp); // After drawing the game view

    // After rendering the world
    virtual void OnRenderView(CWorld &wo, CEntity *penViewer, CAnyProjection3D &apr, CDrawPort *pdp);
};

// Networking events
class INetworkEvents : public IAbstractEvents {
  public:
    // Return handlers container
    virtual CPluginInterfaces *GetContainer(void) {
      return &GetPluginAPI()->cNetworkEvents;
    };

    // Upon receiving a packet as a server (returns TRUE if the packet was handled)
    virtual BOOL OnServerPacket(CNetworkMessage &nmMessage, const ULONG ulType);

    // Upon receiving a packet as a client (returns TRUE if the packet was handled)
    virtual BOOL OnClientPacket(CNetworkMessage &nmMessage, const ULONG ulType);
};

// Game events
class IGameEvents : public IAbstractEvents {
  public:
    // Return handlers container
    virtual CPluginInterfaces *GetContainer(void) {
      return &GetPluginAPI()->cGameEvents;
    };

    // After starting the server and loading in the world
    virtual void OnGameStart(void);

    // Before stopping the server
    virtual void OnGameStop(void);

    // After saving a local game
    virtual void OnGameSave(const CTFileName &fnmSave);

    // After loading a local game
    virtual void OnGameLoad(const CTFileName &fnmSave);
};

// World events
class IWorldEvents : public IAbstractEvents {
  public:
    // Return handlers container
    virtual CPluginInterfaces *GetContainer(void) {
      return &GetPluginAPI()->cWorldEvents;
    };

    // After finishing reading the world file
    virtual void OnWorldLoad(CWorld *pwo, const CTFileName &fnmWorld);
};

// Iteration through specific plugin event handlers
#define FOREACHPLUGINHANDLER(_Container, _HandlerType, _Iter) \
  CDynamicContainer<_HandlerType> &cont_##_HandlerType = (CDynamicContainer<_HandlerType> &)_Container; \
  FOREACHINDYNAMICCONTAINER(cont_##_HandlerType, _HandlerType, _Iter)

#endif
