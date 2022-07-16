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

#ifndef CECIL_INCL_PLUGINEVENTS_H
#define CECIL_INCL_PLUGINEVENTS_H

#ifdef PRAGMA_ONCE
  #pragma once
#endif

// Abstract class for plugin events
class IAbstractEvents {
  public:
    // Destructor with automatic unregistering
    virtual ~IAbstractEvents() {
      Unregister();
    };
    
    virtual void Register(void) {};
    virtual void Unregister(void) {};
};

// Main plugin events
class IProcessingEvents : public IAbstractEvents {
  public:
    virtual void OnStep(void); // Every simulation tick for synchronized logic
    virtual void OnFrame(CDrawPort *pdp); // After rendering everything
    
    // Register events
    void Register(void) {
      GetPluginAPI()->cProcessors.Add(this);
    };
    
    // Unregister events
    void Unregister(void) {
      GetPluginAPI()->cProcessors.Remove(this);
    };
};

// Rendering events
class IRenderingEvents : public IAbstractEvents {
  public:
    virtual void OnPreDraw(CDrawPort *pdp); // Before drawing the game view
    virtual void OnPostDraw(CDrawPort *pdp); // After drawing the game view
    
    // After rendering the world
    virtual void OnRenderView(CWorld &wo, CEntity *penViewer, CAnyProjection3D &apr, CDrawPort *pdp);
    
    // Register events
    void Register(void) {
      GetPluginAPI()->cRenderers.Add(this);
    };

    // Unregister events
    void Unregister(void) {
      GetPluginAPI()->cRenderers.Remove(this);
    };
};

// Iteration through specific plugin event handlers
#define FOREACHPLUGINEVENT(_Container, _HandlerType, _Iter) \
  CDynamicContainerIterator<IAbstractEvents> iter_##_HandlerType(_Container); \
  for (_HandlerType *_Iter = (_HandlerType *)&*iter_##_HandlerType; \
      !iter_##_HandlerType.IsPastEnd(); \
       iter_##_HandlerType.MoveToNext(), _Iter = (_HandlerType *)&*iter_##_HandlerType)

#endif
