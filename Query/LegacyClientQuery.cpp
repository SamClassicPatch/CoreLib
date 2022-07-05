/* Copyright (c) 2002-2012 Croteam Ltd. 
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

#define MSPORT      28900
#define BUFFSZ      8192
#define BUFFSZSTR   4096

#define CLEANMSSRUFF1       closesocket(_sock); \
                            WSACleanup();

#define CLEANMSSRUFF2       if (cResponse) free (cResponse); \
                            closesocket(_sock); \
                            WSACleanup();

#define CHK_BUFFSTRLEN if ((iLen < 0) || (iLen > BUFFSZSTR)) { \
                        CPrintF("\n" \
                            "Error: the used buffer is smaller than how much needed (%d < %d)\n" \
                            "\n", iLen, BUFFSZSTR); \
                            if (cMsstring) free (cMsstring); \
                            closesocket(_sock); \
                            WSACleanup(); \
                        }

extern unsigned char *gsseckey(u_char *secure, u_char *key, int enctype);
extern u_int resolv(char *host);

#define PCK         "\\gamename\\%s" \
                    "\\enctype\\%d" \
                    "\\validate\\%s" \
                    "\\final\\" \
                    "\\queryid\\1.1" \
                    "\\list\\cmp" \
                    "\\gamename\\%s" \
                    "\\gamever\\1.05" \
                    "%s%s" \
                    "\\final\\"

// [Cecil] Use query data here
using namespace QueryData;

extern void _initializeWinsock(void);
extern void _uninitWinsock();
extern void _sendPacket(const char* szBuffer);
extern void _sendPacket(const char* pubBuffer, INDEX iLen);
extern void _sendPacketTo(const char* szBuffer, sockaddr_in* addsin);
extern void _sendPacketTo(const char* pubBuffer, INDEX iLen, sockaddr_in* sin);
extern void _setStatus(const CTString &strStatus);

static void _LocalSearch()
{
  // make sure that there are no requests still stuck in buffer
  ga_asrRequests.Clear();

  // we're not a server
  _bServer = FALSE;
  _pNetwork->ga_strEnumerationStatus = ".";

  WORD     _wsaRequested;
  WSADATA  wsaData;
  PHOSTENT _phHostinfo;
  ULONG    _uIP,*_pchIP = &_uIP;
  USHORT   _uPort,*_pchPort = &_uPort;
  INT      _iLen;
  char     _cName[256],*_pch,_strFinal[8] = {0};

  struct in_addr addr;

  // make the buffer that we'll use for packet reading
  if (_szIPPortBufferLocal != NULL) {
     return;
  }
  _szIPPortBufferLocal = new char[1024];

  // start WSA
  _wsaRequested = MAKEWORD( 2, 2 );
  if (WSAStartup(_wsaRequested, &wsaData) != 0) {
    CPrintF("Error initializing winsock!\n");
    if (_szIPPortBufferLocal != NULL) {
      delete[] _szIPPortBufferLocal;
    }
    _szIPPortBufferLocal = NULL;
    _uninitWinsock();
    _bInitialized = FALSE;
    _pNetwork->ga_bEnumerationChange = FALSE;
    _pNetwork->ga_strEnumerationStatus = "";
    WSACleanup();

    return;
  }

  _pch = _szIPPortBufferLocal;
  _iLen = 0;
  strcpy(_strFinal,"\\final\\");

  if (gethostname ( _cName, sizeof(_cName)) == 0)
  {
    if ((_phHostinfo = gethostbyname(_cName)) != NULL)
    {
      int _iCount = 0;
      while(_phHostinfo->h_addr_list[_iCount])
      {
        
        addr.s_addr = *(u_long *) _phHostinfo->h_addr_list[_iCount];
        _uIP = htonl(addr.s_addr);
       
        //CPrintF("%lu\n", _uIP);
        
        for (UINT uPort = 25601; uPort < 25622; ++uPort){
          _uPort = htons(uPort);
          memcpy(_pch,_pchIP,4);
          _pch  +=4;
          _iLen +=4;
          memcpy(_pch,_pchPort,2);
          _pch  +=2;
          _iLen +=2;
        }
        ++_iCount;
      }

      memcpy(_pch,_strFinal, 7);
      _pch  +=7;
      _iLen +=7;
      _pch[_iLen] = 0x00;
    }
  }

  _iIPPortBufferLocalLen = _iLen;

  _bActivatedLocal = TRUE;
  _bInitialized = TRUE;
  _initializeWinsock();
}

void CLegacyQuery::EnumTrigger(BOOL bInternet)
{
  // Local Search with Legacy Protocol
  if (!bInternet) {
    _LocalSearch();
    return;
	
  // Internet Search
  } else {

    // make sure that there are no requests still stuck in buffer
    ga_asrRequests.Clear();
    // we're not a server
    _bServer = FALSE;
    _pNetwork->ga_strEnumerationStatus = ".";

    struct  sockaddr_in peer;

    SOCKET  _sock               = NULL;
    u_int   uiMSIP;
    int     iErr,
            iLen,
            iDynsz,
            iEnctype             = 0;
    u_short usMSport             = MSPORT;

    u_char  ucGamekey[]          = {SERIOUSSAMKEY},
            ucGamestr[]          = {SERIOUSSAMSTR},
            *ucSec               = NULL,
            *ucKey               = NULL;

    char    *cFilter             = "",
            *cWhere              = "",
            cMS[128]             = {0},
            *cResponse           = NULL,
            *cMsstring           = NULL,
            *cSec                = NULL;


    strcpy(cMS, ms_strMSLegacy);

    WSADATA wsadata;
    if (WSAStartup(MAKEWORD(2,2), &wsadata) != 0) {
        CPrintF("Error initializing winsock!\n");
        return;
    }

    /* Open a socket and connect to the Master server */

    peer.sin_addr.s_addr = uiMSIP = resolv(cMS);
    peer.sin_port        = htons(usMSport);
    peer.sin_family      = AF_INET;

    _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (_sock < 0) {
        CPrintF("Error creating TCP socket!\n");
        WSACleanup();
        return;
    }
    if (connect(_sock, (struct sockaddr *)&peer, sizeof(peer)) < 0) {
        CPrintF("Error connecting to TCP socket!\n");
        CLEANMSSRUFF1;
        return;
    }

    /* Allocate memory for a buffer and get a pointer to it */

    cResponse = (char*) malloc(BUFFSZSTR + 1);
    if (!cResponse) {
        CPrintF("Error initializing memory buffer!\n");
        CLEANMSSRUFF1;
        return;
    }

    // Master Server should respond to the game.
    /* Reading response from Master Server - returns the string with the secret key */

    iLen = 0;
    iErr = recv(_sock, (char*)cResponse + iLen, BUFFSZSTR - iLen, 0);
    if (iErr < 0) {
        CPrintF("Error reading from TCP socket!\n");
        CLEANMSSRUFF2;
        return;
    }

    iLen += iErr;
    cResponse[iLen] = 0x00; // Terminate the response.

    /* Allocate memory for a buffer and get a pointer to it */

    ucSec = (u_char*) malloc(BUFFSZSTR + 1);
    if (!ucSec) {
        CPrintF("Error initializing memory buffer!\n");
        CLEANMSSRUFF2;
        return;
    }
    memcpy ( ucSec, cResponse,  BUFFSZSTR);
    ucSec[iLen] = 0x00;

    /* Geting the secret key from a string */

    cSec = strstr(cResponse, "\\secure\\");
    if (!cSec) {
        CPrintF("Not valid master server response!\n");
        CLEANMSSRUFF2;
        return;
    } else {
        ucSec  += 15;

    /* Creating a key for authentication (Validate key) */

        ucKey = gsseckey(ucSec, ucGamekey, iEnctype);
    }

    ucSec -= 15;
    if (cResponse) free (cResponse);
    if (ucSec) free (ucSec);

    /* Generate a string for the response (to Master Server) with the specified (Validate ucKey) */

    cMsstring = (char*) malloc(BUFFSZSTR + 1);
    if (!cMsstring) {
        CPrintF("Not valid master server response!\n");
        CLEANMSSRUFF1;
        return;
    }

    iLen = _snprintf(
        cMsstring,
        BUFFSZSTR,
        PCK,
        ucGamestr,
        iEnctype,
        ucKey,
        ucGamestr,
        cWhere,
        cFilter);

    /* Check the buffer */

    CHK_BUFFSTRLEN;

    /* The string sent to master server */

    if (send(_sock,cMsstring, iLen, 0) < 0){
        CPrintF("Error reading from TCP socket!\n");
        if (cMsstring) free (cMsstring);
        CLEANMSSRUFF1;
        return;
    }
    if (cMsstring) free (cMsstring);

    /* Allocate memory for a buffer and get a pointer to it */

    if (_szIPPortBuffer ) {
        CLEANMSSRUFF1;
        return;
    };

    _szIPPortBuffer = (char*) malloc(BUFFSZ + 1);
    if (!_szIPPortBuffer) {
        CPrintF("Error reading from TCP socket!\n");
        CLEANMSSRUFF1;
        return;
    }
    iDynsz = BUFFSZ;

    /* The received encoded data after sending the string (Validate key) */

    iLen = 0;
    while((iErr = recv(_sock, _szIPPortBuffer + iLen, iDynsz - iLen, 0)) > 0) {
        iLen += iErr;
        if (iLen >= iDynsz) {
            iDynsz += BUFFSZ;
            _szIPPortBuffer = (char*)realloc(_szIPPortBuffer, iDynsz);
            if (!_szIPPortBuffer) {
                CPrintF("Error reallocation memory buffer!\n");
                if (_szIPPortBuffer) free (_szIPPortBuffer);
                CLEANMSSRUFF1;
                return;
            }
        }
    }

    CLEANMSSRUFF1;
    _iIPPortBufferLen = iLen;

    _bActivated = TRUE;
    _bInitialized = TRUE;
    _initializeWinsock();
     
  }
}

void CLegacyQuery::EnumUpdate(void)
{
  if (_bActivated)
  {
    HANDLE  _hThread;
    DWORD   _dwThreadId;

    _hThread = CreateThread(NULL, 0, _MS_Thread, 0, 0, &_dwThreadId);
    if (_hThread != NULL) {
      CloseHandle(_hThread);
    }
    _bActivated = FALSE;		
  }

  if (_bActivatedLocal)
  {
    HANDLE  _hThread;
    DWORD   _dwThreadId;

    _hThread = CreateThread(NULL, 0, _LocalNet_Thread, 0, 0, &_dwThreadId);
    if (_hThread != NULL) {
      CloseHandle(_hThread);
    }

    _bActivatedLocal = FALSE;		
  }	
}

static void ParseStatusResponse(sockaddr_in &_sinClient, BOOL bIgnorePing)
{
  CTString strPlayers;
  CTString strMaxPlayers;
  CTString strLevel;
  CTString strGameType;
  CTString strVersion;
  CTString strGameName;
  CTString strSessionName;

  CTString strGamePort;
  CTString strServerLocation;
  CTString strGameMode;
  CTString strActiveMod;

  CHAR* pszPacket = _szBuffer + 1; // we do +1 because the first character is always '\', which we don't care about.

  BOOL bReadValue = FALSE;
  CTString strKey;
  CTString strValue;

  while(*pszPacket != 0)
  {
    switch (*pszPacket)
    {
      case '\\': {
        if (strKey != "gamemode") {
          if (bReadValue) {
            // we're done reading the value, check which key it was
            if (strKey == "gamename") {
              strGameName = strValue;
            } else if (strKey == "gamever") {
              strVersion = strValue;
            } else if (strKey == "location") {
              strServerLocation = strValue;
            } else if (strKey == "hostname") {
              strSessionName = strValue;
            } else if (strKey == "hostport") {
              strGamePort = strValue;
            } else if (strKey == "mapname") {
              strLevel = strValue;
            } else if (strKey == "gametype") {
              strGameType = strValue;
            } else if (strKey == "activemod") {
              strActiveMod = strValue;
            } else if (strKey == "numplayers") {
              strPlayers = strValue;
            } else if (strKey == "maxplayers") {
              strMaxPlayers = strValue;
            } else {
              //CPrintF("Unknown MSLegacy parameter key '%s'!\n", strKey);
            }
            // reset temporary holders
            strKey = "";
            strValue = "";
          }
        }
        bReadValue = !bReadValue;
      } break;

      default: {
        // read into the value or into the key, depending where we are
        if (bReadValue) {
          strValue.InsertChar(strValue.Length(), *pszPacket);
        } else {
          strKey.InsertChar(strKey.Length(), *pszPacket);
        }
      } break;
    }

    // move to next character
    pszPacket++;
  }

  // check if we still have a maxplayers to back up
  if (strKey == "gamemode") {
      strGameMode = strValue;
  }

  if (strActiveMod != "") {
      strGameName = strActiveMod;
  }

  __int64 tmPing = -1; // [Cecil] 'long long' -> '__int64'
  // find the request in the request array
  for (INDEX i=0; i<ga_asrRequests.Count(); i++) {
      CServerRequest &req = ga_asrRequests[i];
      if (req.sr_ulAddress == _sinClient.sin_addr.s_addr && req.sr_iPort == _sinClient.sin_port) {
          tmPing = _pTimer->GetHighPrecisionTimer().GetMilliseconds() - req.sr_tmRequestTime;
          ga_asrRequests.Delete(&req);
          break;
      }
  }
  
  if (bIgnorePing) {
    tmPing = 0;
  }

  if (bIgnorePing || (tmPing > 0 && tmPing < 2500000))
  {
    // insert the server into the serverlist
    CNetworkSession &ns = *new CNetworkSession;
    _pNetwork->ga_lhEnumeratedSessions.AddTail(ns.ns_lnNode);

    // add the server to the serverlist
    ns.ns_strSession = strSessionName;
    ns.ns_strAddress = inet_ntoa(_sinClient.sin_addr) + CTString(":") + CTString(0, "%d", htons(_sinClient.sin_port) - 1);
    ns.ns_tmPing = (tmPing / 1000.0f);
    ns.ns_strWorld = strLevel;
    ns.ns_ctPlayers = atoi(strPlayers);
    ns.ns_ctMaxPlayers = atoi(strMaxPlayers);
    ns.ns_strGameType = strGameType;
    ns.ns_strMod = strGameName;
    ns.ns_strVer = strVersion;
  }
}

DWORD WINAPI _MS_Thread(LPVOID lpParam)
{
  SOCKET _sockudp = NULL;
  struct _sIPPort {
    UBYTE bFirst;
    UBYTE bSecond;
    UBYTE bThird;
    UBYTE bFourth;
    USHORT iPort;
  };

  _setStatus("");
  _sockudp = socket(AF_INET, SOCK_DGRAM, 0);

  if (_sockudp == INVALID_SOCKET){
    WSACleanup();
    return -1;
  }

  _sIPPort* pServerIP = (_sIPPort*)(_szIPPortBuffer);
  while(_iIPPortBufferLen >= 6)
  {
    if (!strncmp((char *)pServerIP, "\\final\\", 7)) {
      break;
    }

    _sIPPort ip = *pServerIP;

    CTString strIP;
    strIP.PrintF("%d.%d.%d.%d", ip.bFirst, ip.bSecond, ip.bThird, ip.bFourth);

    sockaddr_in sinServer;
    sinServer.sin_family = AF_INET;
    sinServer.sin_addr.s_addr = inet_addr(strIP);
    sinServer.sin_port = ip.iPort;

    // insert server status request into container
    CServerRequest &sreq = ga_asrRequests.Push();
    sreq.sr_ulAddress = sinServer.sin_addr.s_addr;
    sreq.sr_iPort = sinServer.sin_port;
    sreq.sr_tmRequestTime = _pTimer->GetHighPrecisionTimer().GetMilliseconds();

    // send packet to server
    sendto(_sockudp,"\\status\\",8,0,
        (sockaddr *) &sinServer, sizeof(sinServer));

    sockaddr_in _sinClient;
    int _iClientLength = sizeof(_sinClient);

    fd_set readfds_udp;                         // declare a read set
    struct timeval timeout_udp;                 // declare a timeval for our timer
    int iRet = -1;

    FD_ZERO(&readfds_udp);                      // zero out the read set
    FD_SET(_sockudp, &readfds_udp);                // add socket to the read set
    timeout_udp.tv_sec = 0;                     // timeout = 0 seconds
    timeout_udp.tv_usec = 50000;               // timeout += 0.05 seconds
    int _iN = select(_sockudp + 1, &readfds_udp, NULL, NULL, &timeout_udp);

    if (_iN > 0)
    {
      /** do recvfrom stuff **/
      iRet =  recvfrom(_sockudp, _szBuffer, 2048, 0, (sockaddr*)&_sinClient, &_iClientLength);
      FD_CLR(_sockudp, &readfds_udp);

      if (iRet != -1 && iRet > 100 && iRet != SOCKET_ERROR) {
        // null terminate the buffer
        _szBuffer[iRet] = 0;
        char *sPch = NULL;
        sPch = strstr(_szBuffer, "\\gamename\\" SERIOUSSAMSTR "\\");

        if (!sPch) {
          CPrintF("Unknown query server response!\n");
          return -1;
        } else {
          ParseStatusResponse(_sinClient, FALSE);
        }

      } else {
        // find the request in the request array
        for (INDEX i = 0; i < ga_asrRequests.Count(); i++)
        {
          CServerRequest &req = ga_asrRequests[i];
          if (req.sr_ulAddress == _sinClient.sin_addr.s_addr && req.sr_iPort == _sinClient.sin_port) {
            ga_asrRequests.Delete(&req);
            break;
          }
        }
      }
    }

    pServerIP++;
    _iIPPortBufferLen -= 6;
  }

  if (_szIPPortBuffer) free (_szIPPortBuffer);
  _szIPPortBuffer = NULL;

  closesocket(_sockudp);
  _uninitWinsock();
  _bInitialized = FALSE;
  _pNetwork->ga_bEnumerationChange = FALSE;
  WSACleanup();

  return 0;
}

DWORD WINAPI _LocalNet_Thread(LPVOID lpParam)
{
  SOCKET _sockudp = NULL;
  struct _sIPPort {
    UBYTE bFirst;
    UBYTE bSecond;
    UBYTE bThird;
    UBYTE bFourth;
    USHORT iPort;
  };

  _sockudp = socket(AF_INET, SOCK_DGRAM, 0);

  if (_sockudp == INVALID_SOCKET)
  {
    WSACleanup();
    _pNetwork->ga_strEnumerationStatus = "";

    if (_szIPPortBufferLocal != NULL) {
      delete[] _szIPPortBufferLocal;
    }

    _szIPPortBufferLocal = NULL;		
    return -1;
  }

  _sIPPort* pServerIP = (_sIPPort*)(_szIPPortBufferLocal);
  
  int optval = 1;
  if (setsockopt(_sockudp, SOL_SOCKET, SO_BROADCAST, (char *)&optval, sizeof(optval)) != 0)
  {
    return -1;
  }

  struct   sockaddr_in saddr;
  saddr.sin_family = AF_INET;
  saddr.sin_addr.s_addr = 0xFFFFFFFF;
  
  unsigned short startport = 25601;
  unsigned short endport =  startport + 20;
  
	for (int i = startport ; i <= endport ; i += 1)
	{
    saddr.sin_port = htons(i);
    sendto(_sockudp, "\\status\\", 8, 0, (sockaddr *) &saddr, sizeof(saddr));
  }

  //while(_iIPPortBufferLocalLen >= 6)
  {
    /*if (!strncmp((char *)pServerIP, "\\final\\", 7)) {
      break;
    }*/

    _sIPPort ip = *pServerIP;

    CTString strIP;
    strIP.PrintF("%d.%d.%d.%d", ip.bFourth, ip.bThird, ip.bSecond, ip.bFirst);

    /*
    sockaddr_in sinServer;
    sinServer.sin_family = AF_INET;
    sinServer.sin_addr.s_addr = inet_addr(strIP);
    sinServer.sin_port = ip.iPort;
    */

    // insert server status request into container
    /*
    CServerRequest &sreq = ga_asrRequests.Push();
    sreq.sr_ulAddress = sinServer.sin_addr.s_addr;
    sreq.sr_iPort = sinServer.sin_port;
    sreq.sr_tmRequestTime = _pTimer->GetHighPrecisionTimer().GetMilliseconds();*/

    // send packet to server
    //sendto(_sockudp,"\\status\\",8,0, (sockaddr *) &sinServer, sizeof(sinServer));

    sockaddr_in _sinClient;
    int _iClientLength = sizeof(_sinClient);

    fd_set readfds_udp;                         // declare a read set
    struct timeval timeout_udp;                 // declare a timeval for our timer
    int iRet = -1;

    FD_ZERO(&readfds_udp);                      // zero out the read set
    FD_SET(_sockudp, &readfds_udp);             // add socket to the read set
    timeout_udp.tv_sec = 0;                     // timeout = 0 seconds
    timeout_udp.tv_usec = 250000; // 0.25 sec //50000;                // timeout += 0.05 seconds

    int _iN = select(_sockudp + 1, &readfds_udp, NULL, NULL, &timeout_udp);
    
    //CPrintF("Received %d answers.\n", _iN);

    if (_iN > 0)
    {
      /** do recvfrom stuff **/
      iRet =  recvfrom(_sockudp, _szBuffer, 2048, 0, (sockaddr*)&_sinClient, &_iClientLength);
      FD_CLR(_sockudp, &readfds_udp);

      if (iRet != -1 && iRet > 100 && iRet != SOCKET_ERROR)
      {
        // null terminate the buffer
        _szBuffer[iRet] = 0;
        char *sPch = NULL;
        sPch = strstr(_szBuffer, "\\gamename\\" SERIOUSSAMSTR "\\");

        if (!sPch) {
          CPrintF("Unknown query server response!\n");

          if (_szIPPortBufferLocal != NULL) {
            delete[] _szIPPortBufferLocal;
          }
          _szIPPortBufferLocal = NULL;               
          WSACleanup();
          return -1;
        } else {
          ParseStatusResponse(_sinClient, TRUE);
        }

      } else {
        // find the request in the request array
        for (INDEX i=0; i<ga_asrRequests.Count(); i++)
        {
          CServerRequest &req = ga_asrRequests[i];
          if (req.sr_ulAddress == _sinClient.sin_addr.s_addr && req.sr_iPort == _sinClient.sin_port) {
            ga_asrRequests.Delete(&req);
            break;
          }
        }
      }
    }

   // pServerIP++;
    //_iIPPortBufferLocalLen -= 6;
  }

  if (_szIPPortBufferLocal != NULL) {
    delete[] _szIPPortBufferLocal;
  }

  _szIPPortBufferLocal = NULL;

  closesocket(_sockudp);
  _uninitWinsock();
  _bInitialized = FALSE;
  _pNetwork->ga_bEnumerationChange = FALSE;
  _pNetwork->ga_strEnumerationStatus = "";
  WSACleanup();

  return 0;
}
