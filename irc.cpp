// GPL License - see http://opensource.org/licenses/gpl-license.php
// Copyright 2006 *nixCoders team - don't forget to credit us

#include "nexus.h"

ircMessage_t ircMessages[IRC_MAX_MESSAGES];

DWORD hIRC(LPVOID) {
	#ifdef NEX_VERBOSE
		nexUtils.log("Starting IRC thread..");
	#endif

	static server_t ircServer = { "irc.rizon.net", 6667 };
	sNex.socket.irc = nexNetwork.serverConnect(&ircServer, SOCK_STREAM);

	// did we fail
	if (sNex.socket.irc == -1) {
		#ifdef NEX_LOGGING
			nexUtils.log("Couldn't connect to irc server!");
		#endif
			nexNetwork.addChannelMsg("^5* ^7Failed while connecting to IRC server");
		return 0;
	}

	// register on irc
	#ifdef NEX_VERBOSE
		nexUtils.log("Registering to irc with nick %s", sNex.ircNick);
	#endif
		if (!nexNetwork.sendText(sNex.socket.irc, "%s %s", IRC_NICK, sNex.ircNick) || !nexNetwork.sendText(sNex.socket.irc, "USER %s 0 * :%s", sNex.ircNick, sNex.ircNick)) {
			#ifdef NEX_VERBOSE
				nexUtils.log("	Failed to register to irc with nick %s", sNex.ircNick);
			#endif
			nexNetwork.addChannelMsg("^5* ^7Failed while registering to IRC server");
			closesocket(sNex.socket.irc);
			sNex.socket.irc = -1;
			return 0;
		}

	// registered sucessfully, chat!
	while (1) {
		// don't loop all the time!
		Sleep(25);

		static char incompleteBuffer[512];
		static int incompleteBufferSize = 0;

		char buffer[1024]; 
		memset(buffer, 0, sizeof(buffer));
		char *bufferPosition = buffer;

		// If we have an old incomplete buffer, add it to the current buffer
		if (incompleteBufferSize > 0) {
			strncpy(buffer, incompleteBuffer, incompleteBufferSize);
			bufferPosition = buffer + incompleteBufferSize;
		}

		int size = recv(sNex.socket.irc, bufferPosition, sizeof(buffer) - incompleteBufferSize - 1, 0);

		// No more need of incomplete buffer now
		incompleteBufferSize = 0;
		
		// Check if socket connected
		if (size == 0) {
			nexNetwork.addChannelMsg(va("^5* ^7You got disconnected from: ^5%s", ircServer.hostname));
			closesocket(sNex.socket.irc);
			sNex.socket.irc = -1;
			return 0;
		}

		// If no data received
		if (size == -1) 
			continue;

		#ifdef NEX_VERBOSE
			nexUtils.log("irc: recv size: %i\n---buffer---\n%s\n---endBuffer---", size, buffer);
		#endif

		char *runningOffset = buffer;
		char *line;
		// Parse data received line by line
		while ((line = nexUtils.strsep(&runningOffset, "\n"))) {
			// If all lines receive complete
			if (!strlen(line))
				break;
			
			// Check for an incomplete line
			if (strcmp(line + strlen(line) - 1, "\r")) {
				#ifdef NEX_VERBOSE
					nexUtils.log("incomplet line: [%s]", line);
				#endif

				// Backup incomplete data
				incompleteBufferSize = strlen(line);
				strncpy(incompleteBuffer, line, incompleteBufferSize);
				break;
			}

			// Remove trailing '\n'
			line[strlen(line) - 1] = '\0';

			#ifdef NEX_VERBOSE
				nexUtils.log("irc: new line: [%s]", line);
			#endif
			
			// Get parts of the line
			char *partsOffset = line;
			char *part1 = nexUtils.strsep(&partsOffset, ":");
			char *part2 = nexUtils.strsep(&partsOffset, ":");
			char *part3 = partsOffset;

			#ifdef NEX_VERBOSE
				nexUtils.log("irc: part1: [%s]", part1);
				nexUtils.log("irc: part2: [%s]", part2);
				nexUtils.log("irc: part3: [%s]", part3);
			#endif

			// Catch ping server request and send 'PONG'
			if (!strcmp(part1, IRC_PING)) {
				nexNetwork.sendText(sNex.socket.irc, "%s:%s", IRC_PONG, part2);
				#ifdef NEX_VERBOSE
					nexUtils.log("irc: server request ping");
				#endif
				continue;
			}

			// Get parts from part2
			partsOffset	= part2;
			char *from = nexUtils.strsep(&partsOffset, " ");
			char *command = nexUtils.strsep(&partsOffset, " ");
			char *to = nexUtils.strsep(&partsOffset, " ");
			char *target = nexUtils.strsep(&partsOffset, " ");
			char *temp;
			while ((temp = nexUtils.strsep(&partsOffset, " ")))
				strcat(target, va(" %s", temp));
		
			// Sanity check
			if (!from || !command)
				continue;

			// Discard host info on nickname
			if (strchr(from, '!'))
				*strchr(from, '!') = '\0';

			// Discard 'CTCP VERSION' message
			if (!strcmp(command, IRC_PRIVMSG) && !strcmp(part3, IRC_CTCP_VERSION))
				continue;

			// Register on the server
			if (!strcmp(command, IRC_REGISTRED)) {
				nexNetwork.addChannelMsg(va("^5* ^7Sucessfully connected to: ^5%s", ircServer.hostname));
				// Join channel
				nexNetwork.sendText(sNex.socket.irc, "%s %s", IRC_JOIN, IRC_BOT_CHANNEL);
				continue;
			}

			#ifdef NEX_VERBOSE
				nexUtils.log("irc: from: [%s]", from);
				nexUtils.log("irc: command: [%s]", command);
				nexUtils.log("irc: to: [%s]", to);
				nexUtils.log("irc: target: [%s]", target);
			#endif

			// New message
			if (!strcmp(command, IRC_PRIVMSG)) {
				// action
				if (!strncmp(part3, IRC_ACTION, strlen(IRC_ACTION))) {
					part3[strlen(part3) - 1] = '\0';
					nexNetwork.addChannelMsg(va("^5* ^7%s %s", from, part3 + strlen(IRC_ACTION)));
				// New message for me
				} else if (!strcmp(to, sNex.ircNick)) {
					nexNetwork.addChannelMsg(va("^0Priv msg from ^7%s^0: ^7%s", from, part3));
				// New message for channel
				} else if (!strcmp(to, IRC_BOT_CHANNEL)) {
					nexNetwork.addChannelMsg(va("^0[^2%s^0]^7 %s", from, part3));
				}
				continue;
			}
			
			// Server send list of names on channel
			if (!strcmp(command, IRC_LIST_NAMES)) {
				nexNetwork.addChannelMsg(va("^0[^2Users %s^0]", IRC_BOT_CHANNEL));
				partsOffset	= part3;
				char *name;
				char *list = " ";
				int count = 0;
				while ((name = nexUtils.strsep(&partsOffset, " "))) {
					#ifdef NEX_VERBOSE
						nexUtils.log("irc: name on channel: [%s]", name);
					#endif
					count++;

					if (strlen(list) > 1)
						list = va("%s ^0[^7%s^0] ", list, name);
					else
						list = va(" ^0[^7%s^0]", name);

					if (!(count % 5)) {
						nexNetwork.addChannelMsg(list);
						list = " ";
					}
				}

				if (strlen(list) > 1)
					nexNetwork.addChannelMsg(list);
				list = " ";

				nexNetwork.addChannelMsg(va("^0[^2Total of ^7%i ^2users on %s^0]", count, IRC_BOT_CHANNEL));
				continue;
			}

			// Someone join the channel
			if (!strcmp(command, IRC_JOIN) && strcmp(from, sNex.ircNick)) {
				#ifdef NEX_VERBOSE
					nexUtils.log("irc: idler %s join channel", from);
				#endif
				nexNetwork.addChannelMsg(va("^0* ^7%s ^5has joined the channel", from));
			// Someone leave or quit the channel
			} else if (!strcmp(command, IRC_PART) || !strcmp(command, IRC_QUIT)) {
				#ifdef NEX_VERBOSE
					nexUtils.log("irc: idler %s leave/quit channel", from);
				#endif
				nexNetwork.addChannelMsg(va("^0* ^7%s ^5has left the channel", from));
			// Someone change his name
			} else if (!strcmp(command, IRC_NICK)) {
				#ifdef NEX_VERBOSE
					nexUtils.log("irc: idler %s change nick to %s", from, part3);
				#endif
				if (!strcmp(from, sNex.ircNick))
					strncpy(sNex.ircNick, part3, sizeof(sNex.ircNick));
				nexNetwork.addChannelMsg(va("^0* ^7%s ^5is now known as ^7%s", from, part3));
			// Server sends channel topic
			} else if (!strcmp(command, IRC_TOPIC)) {
				strncpy(sNex.ircTopic, part3, IRC_TOPIC_SIZE);
				nexNetwork.addChannelMsg(va("^0* ^5Channel topic: ^7%s", sNex.ircTopic));
			// Someone changed channel topic
			} else if (!strcmp(command, IRC_TOPIC_CHANGE)) {
				strncpy(sNex.ircTopic, part3, IRC_TOPIC_SIZE);
				nexNetwork.addChannelMsg(va("^0* ^7%s ^5changed channel topic to: ^7%s", from, sNex.ircTopic));
			// Someone got kicked
			} else if (!strcmp(command, IRC_KICK)) {
				char temp_target[35];
				for (int i = 0, c = 0; i < strlen(target); i++) {
					if (target[i] == ' ')
						break;
					temp_target[c++] = target[i];
				}
				nexNetwork.addChannelMsg(va("^0* ^5%s kicked from the channel by ^5%s^0. ^7Reason: ^5%s", !strcmp(temp_target, sNex.ircNick) ? "You ^7got" : va("%s ^7was", target), from, strlen(part3) ? part3 : " "));
			// Someone changed channel mode(s)
			} else if (!strcmp(command, IRC_MODE) && !strcmp(to, IRC_BOT_CHANNEL)) {
				nexNetwork.addChannelMsg(va("^0* ^5%s ^7sets channel mode: ^5%s", from, target));
			// Get our nick on connect
			} else if (!strcmp(command, IRC_002)) {
				strncpy(sNex.ircNick, to, sizeof(sNex.ircNick));
			}
		}
	}

	return 1;
}

bool CNexNetwork::sendIrcChannel(char *format, ...) {
	char buffer[128];
	
	va_list arglist;
	va_start(arglist, format);
		int size = vsprintf_s(buffer, sizeof(buffer), format, arglist);
	va_end(arglist);

	if (size > strlen(buffer)) {
		#ifdef NEX_LOGGING
			nexUtils.log("ERROR [IRC]: net send irc buffer exceed\n");
		#endif
		return false;
	}

	if (sendText(sNex.socket.irc, "%s %s :%s", IRC_PRIVMSG, IRC_BOT_CHANNEL, buffer)) {
		addChannelMsg(va("^0[^n%s^0]^7 %s", sNex.ircNick, buffer));
		return true;
	}
	return false;
}

void CNexNetwork::addChannelMsg(char *msg) {
	int msgCount = IRC_MAX_MESSAGES - 2;
	while (msgCount >= 0) {
		if (!ircMessages[msgCount].msg || !strlen(ircMessages[msgCount].msg)) {
			msgCount--;
			continue;
		}

		ircMessages[msgCount + 1].msg = ircMessages[msgCount].msg;
		msgCount--;
	}
	ircMessages[0].msg = va(" %s", msg);
}

void CNexNetwork::sendIrcStatus() {
	if (nex.cg_clientNum == -1 || !sNex.ET->serverIP)
		return;

	char *serverInfo = nex.cgs_gameState->stringData + nex.cgs_gameState->stringOffsets[CS_SERVERINFO];
	char *serverName = Q_CleanStr(Info_ValueForKey(serverInfo, "sv_hostname"));

	sendIrcChannel("Playing at [%s %s/%s] [%s:%i] [ %s ]", sNex.ET->version, nex.mod->name, nex.mod->version, nex.server.hostname, nex.server.port, serverName);
	sendIrcChannel("[Nexus v%s] My nick: [ %s ] | Total players: %i | Refs online: %i", NEX_VERSION, strlen(Q_CleanStr(nex.client[nex.cg_clientNum].name)) ? Q_CleanStr(nex.client[nex.cg_clientNum].name) : " ", nex.stats.totalPlayers, nex.stats.refCount);
}

void CNexNetwork::saveIrcNickToConfig() {
	CIniWriter config(va("%s%s", sNex.path, NEX_INI));
	config.WriteString("Nexus", "ircnick", sNex.ircNick + strlen(IRC_BOT_PREFIX));
}

/*
===============================
IRC related console commands
===============================
*/

void sendCommand (void) {
	if (nexSyscall.UI_Argc() == 1) {
		nexEngine.gameMessage(true, "Send a message to the irc channel");
		nexEngine.gameMessage(true, va("Usage: \\%s_irc_send your_message_here", sNex.prefix));
		return;
	}
	
	// Not connected
	if (sNex.socket.irc == -1) {
		nexEngine.gameMessage(true, "irc: not connected");
		return;
	}

	// Get all arguments
	char buffer[1024];
	
	strncpy(buffer, nexSyscall.UI_Argv(1), sizeof(buffer));

	int count;
	for (count = 2; count < nexSyscall.UI_Argc(); count++) {
		strncat(buffer, " ", sizeof(buffer));
		strncat(buffer, nexSyscall.UI_Argv(count), sizeof(buffer));
	}

	if (strlen(buffer))
		nexNetwork.sendIrcChannel(buffer);
}

void nickCommand (void) {
	if (nexSyscall.UI_Argc() == 1) {
		nexEngine.gameMessage(true, "Change nick of irc bot");
	}
	if ((nexSyscall.UI_Argc() == 1) || (nexSyscall.UI_Argc() > 2)) {
		nexEngine.gameMessage(true, va("Usage: %s_irc_nick", sNex.prefix));
		return;
	}

	char ircNick[sizeof(sNex.ircNick)];
	sprintf_s(ircNick, sizeof(ircNick), "%s%s", IRC_BOT_PREFIX, nexSyscall.UI_Argv(1));

	if (sNex.socket.irc != -1)
		nexNetwork.sendText(sNex.socket.irc, "NICK %s", ircNick);
	else
		nexEngine.gameMessage(true, "irc: not connected");

	free(ircNick);
}

void messageSendCommand (void) {
	char messageText[256];
	int	messageType;
	
	char *cmd;
	char send[512];

	// get values
	nexSyscall.Cvar_VariableStringBuffer( "cg_messageType", messageText, 256 );
	messageType = atoi( messageText );
	nexSyscall.Cvar_VariableStringBuffer( "cg_messageText", messageText, 256 );

	switch (messageType) {
		case 4:
			cmd = va("%s_irc_send", sNex.prefix);
			break;
		case 3:
			cmd = "say_buddy";
			break;
		case 2:
			cmd = "say_team";
			break;
		default:
			cmd = "say";
			break;
	}

	// reset values
	orig_syscall(UI_CMD_EXECUTETEXT, EXEC_APPEND, "cg_messageText \"\"\n");
	orig_syscall(UI_CMD_EXECUTETEXT, EXEC_APPEND, "cg_messageType \"\"\n");

	// don't send empty messages
	if( messageText[ 0 ] == '\0' )
		return;

	sprintf_s(send, sizeof(send), "%s \"%s\"\n", cmd, messageText);

	orig_syscall(UI_CMD_EXECUTETEXT, EXEC_APPEND, send);
}

void messageModePopup (void) {
	char *cmd = "messageMode\n";
	orig_syscall(UI_CMD_EXECUTETEXT, EXEC_APPEND, cmd);
	cmd = "cg_messageType 4\n";
	orig_syscall(UI_CMD_EXECUTETEXT, EXEC_APPEND, cmd);
}

void CNexNetwork::registerIrcCommands() {
	// Replace existing messageSend cmd to work with IRC msgs
	orig_Cmd_AddCommand("messageSend", &messageSendCommand);
	orig_Cmd_AddCommand(va("%s%s", sNex.prefix, "_irc_chat"), &messageModePopup);

	orig_Cmd_AddCommand(va("%s%s", sNex.prefix, "_irc_send"), &sendCommand);
	orig_Cmd_AddCommand(va("%s%s", sNex.prefix, "_irc_nick"), &nickCommand);
}
