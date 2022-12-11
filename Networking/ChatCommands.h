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

#ifndef CECIL_INCL_CHATCOMMANDS_H
#define CECIL_INCL_CHATCOMMANDS_H

// Prefix that the chat commands start with
extern CTString ser_strCommandPrefix;

// Chat command structure
struct SChatCommand {
  typedef BOOL (*CCommandFunc)(CTString &strResult, INDEX iClient, const CTString &strArguments);

  const char *strName;
  CCommandFunc pHandler;

  // Default constructor
  SChatCommand() : strName(""), pHandler(NULL)
  {
  };

  // Constructor with a name and a handler
  SChatCommand(const char *strSetName, CCommandFunc pSetHandler) :
    strName(strSetName), pHandler(pSetHandler)
  {
  };
};

// List of chat commands
extern CDynamicContainer<SChatCommand> _cChatCommands;

// Interface for chat commands
class IChatCommands {
  public:
    // Handle chat command from a client
    static BOOL HandleCommand(INDEX iClient, const CTString &strCommand);

    // Register a new chat command
    static void Register(const char *strName, SChatCommand::CCommandFunc pFunction);

    // Unregister a chat command by its name
    static void Unregister(const char *strName);

    // Register default chat commands
    static void RegisterDefaultCommands(void);
};

#endif
