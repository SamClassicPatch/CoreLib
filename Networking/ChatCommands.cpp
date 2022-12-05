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

#include "ChatCommands.h"
#include "NetworkFunctions.h"

// Prefix that the chat commands start with
CTString ser_strCommandPrefix = "!";

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
BOOL IChatCommands::HandleCommand(INDEX iClient, CTString strCommand)
{
  // Try to remove the command prefix
  if (!strCommand.RemovePrefix(ser_strCommandPrefix)) {
    return TRUE;
  }

  // Extract command name
  CTString strCommandName = strCommand;
  INDEX iCutOff = ExtractCommand(strCommandName);

  // Remove command name from the string
  if (iCutOff != -1) {
    strCommand.RemovePrefix(strCommandName);
    strCommand.TrimSpacesLeft();

  // Nothing left
  } else {
    strCommand = "";
  }

  // Go through the commands
  FOREACHINDYNAMICCONTAINER(_cChatCommands, SChatCommand, itcom)
  {
    // Found matching command
    if (itcom->strName == strCommandName) {
      // Process it
      CTString strOut;
      itcom->pHandler(strOut, strCommand);

      // Reply to the client
      INetwork::SendChatToClient(iClient, " ", strOut);

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

// Register default chat commands
void IChatCommands::RegisterDefaultCommands(void) {
  NOTHING;
};