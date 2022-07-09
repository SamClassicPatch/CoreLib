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

#include "StdH.h"

#include "Networking/CommInterface.h"
#include "Networking/NetworkFunctions.h"

// Original function pointers
static void (CCommunicationInterface::*pServerInit)(void) = NULL;
static void (CCommunicationInterface::*pServerClose)(void) = NULL;

class CComIntPatch : public CCommunicationInterface {
  public:
    void P_EndWinsock(void) {
      // Stop master server enumeration
      if (ms_bDebugOutput) {
        CPrintF("CCommunicationInterface::EndWinsock() -> MS_EnumCancel()\n");
      }
      MS_EnumCancel();
      
      // Original function code
      #if SE1_VER != 105
        if (!cci_bWinSockOpen) return;
      #endif

      int iResult = WSACleanup();
      ASSERT(iResult == 0);
      
      #if SE1_VER != 105
        cci_bWinSockOpen = FALSE;
      #endif
    };

    void P_ServerInit(void) {
      // Proceed to the original function
      (this->*pServerInit)();
      
      if (ms_bDebugOutput) {
        CPrintF("CCommunicationInterface::Server_Init_t()\n");
      }

      // Start new master server
      if (GetComm().IsNetworkEnabled())
      {
        if (ms_bDebugOutput) {
          CPrintF("  MS_OnServerStart()\n");
        }
        MS_OnServerStart();
      }
    };

    void P_ServerClose(void) {
      // Proceed to the original function
      (this->*pServerClose)();
      
      if (ms_bDebugOutput) {
        CPrintF("CCommunicationInterface::Server_Close()\n");
      }

      // Stop new master server
      if (_pShell->GetINDEX("ser_bEnumeration"))
      {
        if (ms_bDebugOutput) {
          CPrintF("  MS_OnServerEnd()\n");
        }
        MS_OnServerEnd();
      }
    };
};

// Original function pointer
static void (CMessageDispatcher::*pSendToClient)(INDEX, const CNetworkMessage &) = NULL;

class CMessageDisPatch : public CMessageDispatcher {
  public:
    void P_SendToClientReliable(INDEX iClient, const CNetworkMessage &nmMessage) {
      // Remember message type
      const MESSAGETYPE eMessage = nmMessage.GetType();

      // Proceed to the original function
      (this->*pSendToClient)(iClient, nmMessage);
      
      if (ms_bDebugOutput) {
        CPrintF("CMessageDispatcher::SendToClientReliable(%d)\n", eMessage);
      }

      // Notify master server that a player is connecting
      if (eMessage == MSG_REP_CONNECTPLAYER && _pShell->GetINDEX("ser_bEnumeration"))
      {
        if (ms_bDebugOutput) {
          CPrintF("  MS_OnServerStateChanged()\n");
        }
        MS_OnServerStateChanged();
      }
    };

    // Packet receiving method type
    typedef BOOL (CCommunicationInterface::*CReceiveFunc)(INDEX, void *, SLONG &);

    // Server receives a speciifc packet
    BOOL ReceiveFromClientSpecific(INDEX iClient, CNetworkMessage &nmMessage, CReceiveFunc pFunc) {
      // Receive message in static buffer
      nmMessage.nm_slSize = nmMessage.nm_slMaxSize;

      // Receive using a specific method
      BOOL bReceived = (GetComm().*pFunc)(iClient, (void *)nmMessage.nm_pubMessage, nmMessage.nm_slSize);

      // If there is message
      if (bReceived) {
        // Init the message structure
        nmMessage.nm_pubPointer = nmMessage.nm_pubMessage;
        nmMessage.nm_iBit = 0;

        UBYTE ubType;
        nmMessage.Read(&ubType, sizeof(ubType));
        nmMessage.nm_mtType = (MESSAGETYPE)ubType;

        // Replace default CServer packet processor or return TRUE to process through the original function
        return INetwork::ServerHandle(this, iClient, nmMessage);
      }

      return bReceived;
    };

    // Server receives a packet
    BOOL P_ReceiveFromClient(INDEX iClient, CNetworkMessage &nmMessage) {
      return ReceiveFromClientSpecific(iClient, nmMessage, &CCommunicationInterface::Server_Receive_Unreliable);
    };

    // Server receives a reliable packet
    BOOL P_ReceiveFromClientReliable(INDEX iClient, CNetworkMessage &nmMessage) {
      return ReceiveFromClientSpecific(iClient, nmMessage, &CCommunicationInterface::Server_Receive_Reliable);
    };
};

// Original function pointers
static void (CSessionState::*pFlushPredictions)(void) = NULL;
static void (CSessionState::*pProcGameStreamBlock)(CNetworkMessage &) = NULL;

class CSessionStatePatch : public CSessionState {
  public:
    void P_FlushProcessedPredictions(void) {
      // Proceed to the original function
      (this->*pFlushPredictions)();

      // Update server for the master server
      if (GetComm().IsNetworkEnabled() && _pShell->GetINDEX("ser_bEnumeration")) {
        if (ms_bDebugOutput) {
          //CPrintF("CSessionState::FlushProcessedPredictions() -> MS_OnServerUpdate()\n");
        }
        MS_OnServerUpdate();
      }
    };

    // Client processes received packet from the server
    void P_ProcessGameStreamBlock(CNetworkMessage &nmMessage) {
      // Copy the tick to process into tick used for all tasks
      _pTimer->SetCurrentTick(ses_tmLastProcessedTick);

      // If cannot handle custom packet
      if (INetwork::ClientHandle(this, nmMessage)) {
        // Call the original function for standard packets
        (this->*pProcGameStreamBlock)(nmMessage);
      }
    };
};

extern void CECIL_ApplyMasterServerPatch(void) {
  // CCommunicationInterface
  void (CCommunicationInterface::*pEndWindock)(void) = &CCommunicationInterface::EndWinsock;
  NewPatch(pEndWindock, &CComIntPatch::P_EndWinsock, "CCommunicationInterface::EndWinsock()");

  pServerInit = &CCommunicationInterface::Server_Init_t;
  NewPatch(pServerInit, &CComIntPatch::P_ServerInit, "CCommunicationInterface::Server_Init_t()");

  pServerClose = &CCommunicationInterface::Server_Close;
  NewPatch(pServerClose, &CComIntPatch::P_ServerClose, "CCommunicationInterface::Server_Close()");

  // CMessageDispatcher
  pSendToClient = &CMessageDispatcher::SendToClientReliable;
  NewPatch(pSendToClient, &CMessageDisPatch::P_SendToClientReliable, "CMessageDispatcher::SendToClientReliable(...)");

  BOOL (CMessageDispatcher::*pRecFromClient)(INDEX, CNetworkMessage &) = &CMessageDispatcher::ReceiveFromClient;
  NewPatch(pRecFromClient, &CMessageDisPatch::P_ReceiveFromClient, "CMessageDispatcher::ReceiveFromClient(...)");

  BOOL (CMessageDispatcher::*pRecFromClientReliable)(INDEX, CNetworkMessage &) = &CMessageDispatcher::ReceiveFromClientReliable;
  NewPatch(pRecFromClientReliable, &CMessageDisPatch::P_ReceiveFromClientReliable, "CMessageDispatcher::ReceiveFromClientReliable(...)");

  // CSessionState
  pFlushPredictions = &CSessionState::FlushProcessedPredictions;
  NewPatch(pFlushPredictions, &CSessionStatePatch::P_FlushProcessedPredictions, "CSessionState::FlushProcessedPredictions()");

  pProcGameStreamBlock = &CSessionState::ProcessGameStreamBlock;
  NewPatch(pProcGameStreamBlock, &CSessionStatePatch::P_ProcessGameStreamBlock, "CSessionState::ProcessGameStreamBlock(...)");

  // Custom symbols
  _pShell->DeclareSymbol("persistent user CTString ms_strGameAgentMS;",  &ms_strGameAgentMS);
  _pShell->DeclareSymbol("persistent user CTString ms_strMSLegacy;",     &ms_strMSLegacy);
  _pShell->DeclareSymbol("persistent user CTString ms_strDarkPlacesMS;", &ms_strDarkPlacesMS);
  _pShell->DeclareSymbol("persistent user INDEX ms_iProtocol;",          &ms_iProtocol);
  _pShell->DeclareSymbol("persistent user INDEX ms_bDebugOutput;",       &ms_bDebugOutput);

  // Master server protocol types
  static const INDEX iMSLegacy   = E_MS_LEGACY;
  static const INDEX iDarkPlaces = E_MS_DARKPLACES;
  static const INDEX iGameAgent  = E_MS_GAMEAGENT;
  _pShell->DeclareSymbol("const INDEX MS_LEGACY;",     (void *)&iMSLegacy);
  _pShell->DeclareSymbol("const INDEX MS_DARKPLACES;", (void *)&iDarkPlaces);
  _pShell->DeclareSymbol("const INDEX MS_GAMEAGENT;",  (void *)&iGameAgent);
};
