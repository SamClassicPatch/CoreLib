/* Copyright (c) 2022-2024 Dreamy Cecil
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

#include "ChatCommands.h"
#include "Networking/NetworkFunctions.h"
#include "StockCommands.h"
#include "VotingSystem.h"
#include "ClientLogging.h"

// Prefix that the chat commands start with
CTString ser_strCommandPrefix = "!";

// Passwords for authorizing as administrator and operator
CTString ser_strAdminPassword = "";
CTString ser_strOperatorPassword = "";

// List of chat commands
CDynamicContainer<SChatCommand> _cChatCommands;

// Extract command name from the string
static INDEX ExtractCommand(CTString &strCommand) {
  const INDEX ct = strCommand.Length();

  // Parse until the end of the string
  INDEX i;

  for (i = 0; i < ct; i++) {
    char ch = strCommand[i];

    // Anything before and including space is a delimiter
    if (ch <= ' ') break;
  }

  // Same length
  if (i == ct) {
    return -1;
  }

  // Cut off the string at the last parsed character
  strCommand.TrimRight(i);
  return i;
};

// Interface for chat commands
BOOL IChatCommands::HandleCommand(INDEX iClient, const CTString &strCommand)
{
  // Copy full command for extracting arguments
  CTString strArguments = strCommand;

  // Try to remove the command prefix
  if (!strArguments.RemovePrefix(ser_strCommandPrefix)) {
    return TRUE;
  }

  // Extract command name
  CTString strCommandName = strArguments;
  INDEX iCutOff = ExtractCommand(strCommandName);

  // Remove command name from the string
  if (iCutOff != -1) {
    strArguments.RemovePrefix(strCommandName);
    strArguments.TrimSpacesLeft();

  // Nothing left
  } else {
    strArguments = "";
  }

  // Go through the commands
  FOREACHINDYNAMICCONTAINER(_cChatCommands, SChatCommand, itcom)
  {
    // Found matching command
    if (itcom->strName == strCommandName) {
      // Execute it
      CTString strOut = "";
      BOOL bHandled = itcom->pHandler(strOut, iClient, strArguments);

      // Process as a normal chat message upon failure
      if (!bHandled) {
        return TRUE;
      }

      // Reply to the client with the inputted command
      const CTString strReply = strCommand + "\n" + strOut;
      INetwork::SendChatToClient(iClient, "Chat command", strReply);

      // Don't process as a chat message
      return FALSE;
    }
  }

  return TRUE;
};

// Register a new chat command
void IChatCommands::Register(const char *strName, SChatCommand::CCommandFunc pFunction)
{
  // Add new command to the container
  _cChatCommands.Add(new SChatCommand(strName, pFunction));
};

// Unregister a chat command by its name
void IChatCommands::Unregister(const char *strName)
{
  FOREACHINDYNAMICCONTAINER(_cChatCommands, SChatCommand, itcom)
  {
    // Matching command
    if (itcom->strName == strName) {
      _cChatCommands.Remove(itcom);
      delete &*itcom;

      return;
    }
  }
};

// Output log of a specific identity and optionally a character
static void ClientLogInConsole(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  INDEX iIdentity = NEXT_ARG(INDEX);
  INDEX iCharacter = NEXT_ARG(INDEX);

  CTString strLog;

  extern void PrintClientLog(CTString &strResult, INDEX iIdentity, INDEX iCharacter);
  PrintClientLog(strLog, iIdentity, iCharacter);

  CPutString(strLog + "\n");
};

// Delete specific character or an identity as a whole from the log
static void ClientLogDelete(SHELL_FUNC_ARGS) {
  BEGIN_SHELL_FUNC;
  INDEX iIdentity = NEXT_ARG(INDEX);
  INDEX iCharacter = NEXT_ARG(INDEX);

  if (iIdentity < 0 || iIdentity >= _aClientIdentities.Count()) {
    CPutString("Invalid client index!\n");
    return;
  }

  CClientIdentity &ci = _aClientIdentities[iIdentity];

  // Delete entire identity
  if (iCharacter == -1) {
    _aClientIdentities.Delete(&ci);
    return;
  }

  if (iCharacter <= 0 || iCharacter > ci.aCharacters.Count()) {
    CPutString("Invalid character index!\n");
    return;
  }

  // Delete character
  CPlayerCharacter &pc = ci.aCharacters[iCharacter - 1];
  ci.aCharacters.Delete(&pc);
};

// Resave client log
static void ClientLogSave(void) {
  IClientLogging::SaveLog();
};

// Reload client log
static void ClientLogLoad(void) {
  _aClientIdentities.Clear();
  IClientLogging::LoadLog();
};

// Register default chat commands
void IChatCommands::RegisterDefaultCommands(void) {
  Register("map",   &IStockCommands::CurrentMap);
  Register("login", &IStockCommands::PasswordLogin);
  Register("rcon",  &IStockCommands::RemoteConsole);
  Register("save",  &IStockCommands::RemoteSave);
  Register("log",   &IStockCommands::ClientLog);
  Register("ban",   &IStockCommands::BanClient);
  Register("mute",  &IStockCommands::MuteClient);
  Register("kick",  &IStockCommands::KickClient);

  // Voting
  Register("y",       &IVotingSystem::Chat::VoteYes);
  Register("n",       &IVotingSystem::Chat::VoteNo);

  // Local interaction with the client log
  _pShell->DeclareSymbol("user void ClientLog(INDEX, INDEX);", &ClientLogInConsole);
  _pShell->DeclareSymbol("user void ClientLogDelete(INDEX, INDEX);", &ClientLogDelete);
  _pShell->DeclareSymbol("user void ClientLogSave(void);", &ClientLogSave);
  _pShell->DeclareSymbol("user void ClientLogLoad(void);", &ClientLogLoad);
};
