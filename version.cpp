// GPL License - see http://opensource.org/licenses/gpl-license.php
// Copyright 2006 *nixCoders team - don't forget to credit us

#include "nexus.h"

DWORD hVER(LPVOID) {
	#ifdef NEX_VERBOSE
		nexUtils.log("Starting ver thread..");
	#endif

	static server_t verServer = { "chaplja.com", 80 };
	sNex.socket.ver = nexNetwork.serverConnect(&verServer, SOCK_STREAM);

	// did we fail
	if (sNex.socket.ver == -1) {
		#ifdef NEX_LOGGING
			nexUtils.log("Couldn't connect to ver ection check server!");
		#endif
		return 0;
	}

	// request shyt
	bool sent = nexNetwork.sendText(sNex.socket.ver, "GET /latest.php HTTP/1.0");
	if (sent)
		sent = nexNetwork.sendText(sNex.socket.ver, "Host: chaplja.com");
	if (sent)
		sent = nexNetwork.sendText(sNex.socket.ver, va("User-Agent: Nexus%s", NEX_VERSION));
	if (sent)
		sent = nexNetwork.sendText(sNex.socket.ver, "\n\n");

	// ver ection test went fine?
	if (!sent) {
		#ifdef NEX_VERBOSE
			nexUtils.log("Failed while requesting ver ection status");
		#endif
		closesocket(sNex.socket.ver);
		sNex.socket.ver = -1;
		return 0;
	}

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

		int size = recv(sNex.socket.ver, bufferPosition, sizeof(buffer) - incompleteBufferSize - 1, 0);

		// No more need of incomplete buffer now
		incompleteBufferSize = 0;
		
		// Check if socket connected
		if (size == 0) {
			closesocket(sNex.socket.ver);
			sNex.socket.ver = -1;
			return 0;
		}

		// If no data received
		if (size == -1) 
			continue;

		#ifdef NEX_VERBOSE
			nexUtils.log("ver : recv size: %i\n---buffer---\n%s\n---endBuffer---", size, buffer);
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
					nexUtils.log("incomplete line: [%s]", line);
				#endif

				// Backup incomplete data
				incompleteBufferSize = strlen(line);
				strncpy(incompleteBuffer, line, incompleteBufferSize);
				break;
			}

			// Remove trailing '\n'
			line[strlen(line) - 1] = '\0';

			#ifdef NEX_VERBOSE
				nexUtils.log("ver : new line: [%s]", line);
			#endif

			if (!strncmp(line, "Updated: ", strlen("Updated: ")) && strlen(line) == 10) {
				#ifdef NEX_VERBOSE
					nexUtils.log("Updated: %s", line + strlen("Updated: "));
				#endif
				sNex.updated = atoi(line + strlen("Updated: "));
				#ifdef NEX_VERBOSE
					nexUtils.log("sNex.updated: %i", sNex.updated);
				#endif
			}
		}
	}

	return 0;
}
