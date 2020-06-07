// GPL License - see http://opensource.org/licenses/gpl-license.php
// Copyright 2006 *nixCoders team - don't forget to credit us

#include "nexus.h"

DWORD hSPC(LPVOID) {
	#ifdef NEX_VERBOSE
		nexUtils.log("Starting SPC thread..");
	#endif

	while (1) {
		#ifdef NEX_VERBOSE
			nexUtils.log("SPC loop!");
		#endif

		// our buffer
		static char buffer[4096];
		memset(buffer, 0, sizeof(buffer));

		// request speclist every 2 seconds
		Sleep(NET_STATUS_TIMEOUT);

		// if we're not playing, don't connect
		if (nex.cg_clientNum == -1 || !nex.cg_time || !nexValue[VAR_SPECLIST]) {
			#ifdef NEX_VERBOSE
				nexUtils.log("SPC id: %i | time: %i | SKIPPING!", nex.cg_clientNum, nex.cg_time);
			#endif
			continue;
		}

		// connect to the server we're playing
		if (sNex.socket.spc == -1) {
			sNex.socket.spc = nexNetwork.serverConnect(&nex.server, SOCK_DGRAM);
			nexNetwork.sendText(sNex.socket.spc, NET_STATUS_REQUEST);
			#ifdef NEX_VERBOSE
				nexUtils.log("SPC socket: %i", sNex.socket.spc);
			#endif
			continue;
		}

		#ifdef NEX_VERBOSE
			nexUtils.log("SPC gonna do recv()");
		#endif

		// retreive list
		int size = recv(sNex.socket.spc, buffer, sizeof(buffer), 0);

		#ifdef NEX_VERBOSE
			nexUtils.log("SPC received %i bytes", size);
		#endif

		// didn't retreive anything
		if (size == -1)
			continue;

		#ifdef NEX_VERBOSE
			nexUtils.log("--- SPC buffer start ---");
			nexUtils.log(buffer);
			nexUtils.log("--- SPC buffer end ---");
		#endif

		// If valid packet
		if (strncmp(buffer, NET_STATUS_RESPONSE, strlen(NET_STATUS_RESPONSE)) != 0) {
			#ifdef NEX_VERBOSE
				nexUtils.log("WARNING [SPC]: invalid packet");		
			#endif
			continue;
		}

		// get info from packet
		char *str = buffer;

		// Skip header
		str += strlen(NET_STATUS_RESPONSE) + 1;

		// Skip server infos
		str = strchr(str, '\n') + 1;
		
		// Reset spec infos
		nex.stats.specCount = 0;
		memset(nex.stats.specNames, 0, sizeof(nex.stats.specNames));

		#ifdef NEX_VERBOSE
			nexUtils.log("SPC: Parsing all names");
		#endif

		// Get all names
		while (str && (str < buffer + size)) {
			char line[128];
			memset(line, 0, sizeof(line));
			strncpy(line, str, strchr(str, '\n') - str);
				
			char xp[8];
			memset(xp, 0, sizeof(xp));
			strncpy(xp, str, strchr(str, ' ') - str);

			#ifdef NEX_VERBOSE
				nexUtils.log("SPC: my XP: %i | his XP: %i", nex.cg_snap->ps.stats[STAT_XP], atoi(xp));
			#endif
			

			int myxp;
			if (nex.mod->type != MOD_TCE)
				myxp = nex.cg_snap->ps.stats[STAT_XP];
			else
				myxp = nex.cg_snap->ps.persistant[PERS_SCORE];

			if ((myxp == atoi(xp)) && (atoi(xp))) {
				char *start = strchr(line, '"') + 1;
				char *end = strrchr(line, '"') - 1;

				if ((start == NULL) || (end == NULL) || (start == end)) {
					#ifdef NEX_VERBOSE
						nexUtils.log("WARNING [SPC]: invalid player infos");
					#endif
					break;
				}
					
				char name[MAX_QPATH];
				memset(name, 0, sizeof(name));
				strncpy(name, start, end - start + 1);

				#ifdef NEX_VERBOSE
					nexUtils.log("His name: %s", name);
				#endif

				int i = 0;
				for (; i < MAX_CLIENTS; i++) {
					if (nex.client[i].infoValid && !strcmp(nex.client[i].name, name)
							&& (nex.client[i].team == TEAM_SPECTATOR)) {
						strcpy(nex.stats.specNames[nex.stats.specCount], name);
						nex.stats.specCount++;
						break;
					}
				}
			}
				
			// Next line
			str = strchr(str, '\n') + 1;
		}
		closesocket(sNex.socket.spc);
		sNex.socket.spc = -1;
	}

	return 0;
}
