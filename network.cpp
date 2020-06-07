// GPL License - see http://opensource.org/licenses/gpl-license.php
// Copyright 2006 *nixCoders team - don't forget to credit us

#include "nexus.h"

CNexNetwork nexNetwork;

void CNexNetwork::init() {
	static bool inited = false;
	if (!inited) {
		#ifdef NEX_VERBOSE
			nexUtils.log("Initing socket stuff");
		#endif

		// winblows sockets
		WSADATA ws;
		WSAStartup(0x0101, &ws);

		// reset our sockets
		sNex.socket.det = -1;
		sNex.socket.irc = -1;
		sNex.socket.spc = -1;
		sNex.socket.ver = -1;

		// set our irc nick
		srand(999999);
		CIniReader config(va("%s%s", sNex.path, NEX_INI));
		strncpy(sNex.ircNick, va("%s%s", IRC_BOT_PREFIX, config.ReadString("Nexus", "ircnick", va("luser%i", rand() % 100000))), sizeof(sNex.ircNick));

		// init some network threads
		if (sNex.useIrc) {
			memset(&ircMessages, 0, sizeof(ircMessages));
			sNex.hNetworkIRC = CTHREAD(hIRC);
		}
		if (sNex.useDet) {
			sNex.detected = false;
			sNex.hNetworkDET = CTHREAD(hDET);
		}
		if (sNex.useDet) {
			sNex.updated = false;
			sNex.hNetworkVER = CTHREAD(hVER);
		}

		memset(&nex.stats.specNames, 0, sizeof(nex.stats.specNames));
		nex.stats.specCount = 0;
		sNex.hNetworkSPC = CTHREAD(hSPC);

		inited = true;
	}
}

socket_t CNexNetwork::serverConnect(server_t *server, int type) {
	socket_t sock;
	struct sockaddr_in sin;
	struct hostent *he;
	memset(&sin, 0, sizeof(sin));

	// check hostname
	if ((he = gethostbyname(server->hostname)) == NULL) {
		#ifdef NEX_LOGGING
			nexUtils.log("ERROR [NETWORK]: gethostbyname(%s)", server->hostname);
		#endif
		return -1;
	}

	// create socket
	sock = socket(AF_INET, type, 0);
	sin.sin_addr.S_un.S_addr = ((struct in_addr *)(he->h_addr))->s_addr;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(server->port);

	// connect the socket
	int result = connect(sock, (struct sockaddr *) &sin, sizeof(sin));
	if (result == SOCKET_ERROR) {
		#ifdef NEX_LOGGING
			nexUtils.log("ERROR [NETWORK]: can't connect to %s (port: %i)", server->hostname, server->port);
		#endif
		return -1;
	}

	return sock;
}

bool CNexNetwork::sendRaw(socket_t socket, char *buffer, int size) {
	if (send(socket, buffer, size, 0) < 0) {
		#ifdef NEX_VERBOSE
			nexUtils.log("ERROR [NETWORK]: sendRaw error");
		#endif
		return false;
	}

	#ifdef NEX_VERBOSE
		nexUtils.log("--- SEND BUFFER ---");
		nexUtils.log("sendRaw: %s", buffer);
		nexUtils.log("--- END OF SEND BUFFER ---");
	#endif

	return true;
}

bool CNexNetwork::sendText(socket_t socket, char *format, ...) {
	char buffer[512];
	static char *trailing = "\n";
	
	va_list arglist;
	va_start(arglist, format);
		int size = vsprintf_s(buffer, sizeof(buffer) - strlen(trailing), format, arglist);
	va_end(arglist);

	// Sanity check
	if (size > strlen(buffer)) {
		#ifdef NEX_LOGGING
			nexUtils.log("ERROR [NETWORK]: net send buffer exceeded");
		#endif
		return false;
	}

	strncat(buffer, trailing, sizeof(buffer) - strlen(trailing));
	
	return sendRaw(socket, buffer, strlen(buffer));
}

