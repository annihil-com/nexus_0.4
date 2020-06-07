// GPL License - see http://opensource.org/licenses/gpl-license.php
// Copyright 2006 *nixCoders team - don't forget to credit us

#include "nexus.h"

// Hook GAME/ENGINE stuff
CNexHook nexHook;
vmMain_t orig_CG_vmMain;
dllEntry_t orig_CG_dllEntry;
dllEntry_t orig_UI_dllEntry;
syscall_t orig_syscall;
FS_PureServerSetLoadedPaks_t orig_FS_PureServerSetLoadedPaks;
char goodChecksumsPak[BIG_INFO_STRING];
Cmd_AddCommand_t orig_Cmd_AddCommand;
Cvar_Get_t orig_Cvar_Get;

/*
===============================
Some hooking funcs
===============================
*/

void CNexHook::hookSY(bool state) {
	#ifdef NEX_VERBOSE
		nexUtils.log("%sooking System funcs", state ? "H" : "Un-H");
	#endif

	static bool hooked = false;

	if (state && !hooked) {
		orig_LoadLibraryA = (LoadLibraryA_t) DetourFunction((BYTE *) LoadLibraryA, (BYTE *) nex_LoadLibraryA);
	} else if (hooked) {
		DetourRemove((BYTE *) orig_LoadLibraryA, (BYTE *) nex_LoadLibraryA);
	}

	hooked = state;
}

void CNexHook::hookCG(bool state) {
	#ifdef NEX_VERBOSE
		nexUtils.log("%sooking CG", state ? "H" : "Un-H");
	#endif

	if (state && !sNex.cgHooked) {
		if (nex.mod->type == MOD_ETPRO) {
			#ifdef NEX_VERBOSE
				nexUtils.log("Hooking etpro stuff");
			#endif
			nex.orig.CG_ConfigString = (CG_ConfigString_t) DetourFunction((BYTE*) sNex.cgHandle + nex.mod->CG_ConfigString, (BYTE*) nex_CG_ConfigString);
			orig_etproAntiCheat = (etproAntiCheat_t) DetourFunction((BYTE *) sNex.cgHandle + nex.mod->anticheat, (BYTE *) nex_etproAntiCheat);
		} else {
			orig_CG_dllEntry = (dllEntry_t) DetourFunction((BYTE *) GetProcAddress(sNex.cgHandle, "dllEntry"), (BYTE *) nex_CG_dllEntry);
		}
		orig_CG_vmMain = (vmMain_t) DetourFunction((BYTE *) GetProcAddress(sNex.cgHandle, "vmMain"), (BYTE *) nex_CG_vmMain);

		#ifdef NEX_VERBOSE
			nexUtils.log("Hooking the usual functions");
		#endif

		DWORD CG_Trace_off = (DWORD) sNex.cgHandle + nex.mod->CG_Trace;
		if (nex.mod->type != MOD_TCE) {
			nex.orig.CG_FinishWeaponChange = (CG_FinishWeaponChange_t) DetourFunction((BYTE *) sNex.cgHandle + nex.mod->CG_FinishWeaponChange, (BYTE *) nex_CG_FinishWeaponChange);
			nex.orig.CG_Trace = (CG_Trace_t) CG_Trace_off;
		} else {
			nex.orig.CG_Trace = (CG_Trace_t) DetourFunction((BYTE *) CG_Trace_off, (BYTE *) nex_CG_Trace);
		}
		

		nex.orig.CG_Init = (CG_Init_t) DetourFunction((BYTE*) sNex.cgHandle + nex.mod->CG_Init, (BYTE*) nex_CG_Init);
		nex.orig.CG_DrawActiveFrame = (CG_DrawActiveFrame_t) DetourFunction((BYTE*) sNex.cgHandle + nex.mod->CG_DrawActiveFrame, (BYTE*) nex_CG_DrawActiveFrame);
		nex.orig.CG_DamageFeedback = (CG_DamageFeedback_t) DetourFunction((BYTE *) sNex.cgHandle + nex.mod->CG_DamageFeedback, (BYTE *) nex_CG_DamageFeedback);
		nex.orig.CG_EntityEvent = (CG_EntityEvent_t) DetourFunction((BYTE *) sNex.cgHandle + nex.mod->CG_EntityEvent, (BYTE *) nex_CG_EntityEvent);

		sNex.cgHooked = true;
	} else if (!state && sNex.cgHooked) {
		if (nex.mod->type == MOD_ETPRO) {
			DetourRemove((BYTE *) nex.orig.CG_ConfigString, (BYTE *) nex_CG_ConfigString);
			DetourRemove((BYTE *) orig_etproAntiCheat, (BYTE *) nex_etproAntiCheat);
		} else {
			DetourRemove((BYTE *) orig_CG_dllEntry, (BYTE *) nex_CG_dllEntry);
		}

		if (nex.mod->type != MOD_TCE) {
			DetourRemove((BYTE *) nex.orig.CG_FinishWeaponChange, (BYTE *) nex_CG_FinishWeaponChange);
		} else {
			DetourRemove((BYTE *) nex.orig.CG_Trace, (BYTE *) nex_CG_Trace);
		}

		DetourRemove((BYTE *) orig_CG_vmMain, (BYTE *) nex_CG_vmMain);

		DetourRemove((BYTE *) nex.orig.CG_Init, (BYTE *) nex_CG_Init);
		DetourRemove((BYTE *) nex.orig.CG_DrawActiveFrame, (BYTE *) nex_CG_DrawActiveFrame);
		DetourRemove((BYTE *) nex.orig.CG_DamageFeedback, (BYTE *) nex_CG_DamageFeedback);
		DetourRemove((BYTE *) nex.orig.CG_EntityEvent, (BYTE *) nex_CG_EntityEvent);
		sNex.cgHooked = false;
	}

	#ifdef NEX_VERBOSE
		nexUtils.log("Done %sooking CG", state ? "H" : "Un-H");
	#endif
}

void CNexHook::hookET(bool state) {
	#ifdef NEX_VERBOSE
		nexUtils.log("%sooking ET", state ? "H" : "Un-H");
	#endif

	if (state && !sNex.etHooked) {
		sNex.etHooked = true;
		orig_FS_PureServerSetLoadedPaks = (FS_PureServerSetLoadedPaks_t) DetourFunction((BYTE *) sNex.ET->FS_PureServerSetLoadedPaks, (BYTE *) nex_FS_PureServerSetLoadedPaks);
		orig_Cmd_AddCommand = (Cmd_AddCommand_t) DetourFunction((BYTE *) sNex.ET->Cmd_AddCommand, (BYTE *) nex_Cmd_AddCommand);
		orig_Cvar_Get = (Cvar_Get_t) DetourFunction((BYTE *) sNex.ET->Cvar_Get, (BYTE *) nex_Cvar_Get);
	} else if (!state) {
		sNex.etHooked = false;
		DetourRemove((BYTE *) orig_FS_PureServerSetLoadedPaks, (BYTE *) nex_FS_PureServerSetLoadedPaks);
		DetourRemove((BYTE *) orig_Cmd_AddCommand, (BYTE *) nex_Cmd_AddCommand);
		DetourRemove((BYTE *) orig_Cvar_Get, (BYTE *) nex_Cvar_Get);
	}

	#ifdef NEX_VERBOSE
		nexUtils.log("Done %sooking ET", state ? "H" : "Un-H");
	#endif
}

pack_t *CNexHook::getPack(char *filename) {
	searchpath_t *browse = *(searchpath_t **)sNex.ET->fs_searchpaths;
	for (; browse; browse = browse->next)
		if (browse->pack && strstr(browse->pack->pakFilename, filename))
			return browse->pack;
	return NULL;
}

void CNexHook::addCommands() {
	static bool done = false;
	if (!done) {
		#ifdef NEX_VERBOSE
			nexUtils.log("Adding console commands");
		#endif

		nexNetwork.registerIrcCommands();
		done = true;
		#ifdef NEX_VERBOSE
			nexUtils.log("Done adding console commands");
		#endif
	}
}

/*
===============================
Hooked functions
===============================
*/

void __cdecl nex_CG_dllEntry(int (*syscallptr)(int arg,... )) {
	#ifdef NEX_LOGGING
		nexUtils.log("CG_dllEntry() - syscallptr: 0x%x | Redirecting CG syscall", syscallptr);
	#endif

	orig_CG_dllEntry(nex_CG_syscall);
}

void __cdecl nex_UI_dllEntry(int (*syscallptr)(int arg,... )) {
	#ifdef NEX_LOGGING
		nexUtils.log("UI_dllEntry() - syscallptr: 0x%x | Redirecting UI syscall", syscallptr);
	#endif

	orig_UI_dllEntry(nex_UI_syscall);
}

int __cdecl nex_CG_vmMain(int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10, int arg11) {
	switch (command) {
		case CG_SHUTDOWN: {
			int result = orig_CG_vmMain(command, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11);
			nexEngine.CG_Shutdown();
			return result;
		}
		default:
			break;
	}

	return orig_CG_vmMain(command, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11);
}

int __cdecl nex_CG_syscall(int command, ...) {
	// Get all arguments to send them to the original syscall
	int arg[10];
	va_list arglist;
	va_start(arglist, command);
	int count = 0;
	for (; count < 10; count++)
		arg[count] = va_arg(arglist, int);
	va_end(arglist);

	bool callOriginal = true;

	if (command == CG_GETSNAPSHOT)
		nex.cg_snap = (snapshot_t *) arg[1];
	else if (command == CG_CVAR_REGISTER) {
		if (arg[3] & CVAR_CHEAT) {
			#ifdef NEX_VERBOSE
				nexUtils.log("CG: Unlocking cheat cvar: %s", arg[1]);
			#endif

			arg[3] &= ~CVAR_CHEAT;
		}
	}

	if (nex.cg_snap) {
		switch (command) {
			case CG_R_ADDREFENTITYTOSCENE: {
				nexEngine.CG_R_AddRefEntityToScene((refEntity_t *) arg[0]);
				break;
			}
			case CG_S_UPDATEENTITYPOSITION: {
				nexEngine.CG_S_UpdateEntityPosition(arg[0], *(vec3_t *) arg[1]);
				break;
			}
			case CG_R_DRAWSTRETCHPIC: {
				callOriginal = nexEngine.CG_R_DrawStretchPic(*(float *) &arg[0], *(float *) &arg[1], *(float *) &arg[2], *(float *) &arg[3], *(float *) &arg[4], *(float *)&arg[5], *(float *)&arg[6], *(float *)&arg[7], (qhandle_t)arg[8]);
				break;
			}
			case CG_R_ADDPOLYTOSCENE: {
				callOriginal = nexEngine.CG_R_AddPolyToScene(arg[0], (polyVert_t*) arg[2]);
				break;
			}
			case CG_R_RENDERSCENE: {
				callOriginal = nexEngine.CG_R_RenderScene((refdef_t *) arg[0]);
				break;
			}
			case CG_SETUSERCMDVALUE: {
				if (nexValue[VAR_NOWEAPONZOOM])
					*(float *) &arg[2] = 1.0f;
				break;
			}
			default:
				break;
		}
	}

	if (!callOriginal)
		return 0;

	int result = orig_syscall(command, arg[0], arg[1], arg[2], arg[3], arg[4], arg[5], arg[6], arg[7], arg[8], arg[9]);

	switch (command) {
		case CG_R_REGISTERSHADER:
		case CG_R_REGISTERSHADERNOMIP:
		case CG_R_REGISTERMODEL:
		case CG_R_REGISTERSKIN:
		case CG_S_REGISTERSOUND: {
			char *name = (char *)arg[0];

			if (!name || !strlen(name))
				break;

			if (!strcmp(name, "white"))
				nex.media.whiteShader = result;
			else if (!strcmp(name, "gfx/misc/reticlesimple"))
				nex.media.reticleShaderSimple = result;
			else if (!strcmp(name, "smokePuff"))
				nex.media.smokePuffshader = result;
			else if (!strcmp(name, "gfx/misc/binocsimple"))
				nex.media.binocShaderSimple = result;
			else if (!strcmp(name, "models/players/hud/head.md3"))
				nex.media.hHead = result;
			else if (!strcmp(name, "models/players/hud/glasses.md3"))
				nex.media.hGlasses = result;
			else if (!strcmp(name, "ui/assets/3_cursor3"))
				nex.media.cursorIcon = result;
			else if (!strcmp(name, "sound/menu/select.wav"))
				nex.media.limboSelect = result;

			break;
		}
		case CG_GETGAMESTATE: {
			nex.cgs_gameState = (gameState_t *) arg[0];
			char *info;
			// Get client infos
			int client = 0;
			nex.stats.refCount = 0;
			nex.stats.totalPlayers = 0;
			for (; client < MAX_CLIENTS; client++) {
				int offset = nex.cgs_gameState->stringOffsets[client + CS_PLAYERS];
				if (offset) {
					// Set client as valid
					nex.client[client].infoValid = true;
					// Set name and cleanname
					info = Info_ValueForKey((char *)(nex.cgs_gameState->stringData + offset), "n");
					strncpy(nex.client[client].name, info, sizeof(nexClientInfo_t));
					strncpy(nex.client[client].cleanName, Q_CleanStr(info), sizeof(nexClientInfo_t));
					// Get team
					info = Info_ValueForKey((char *)(nex.cgs_gameState->stringData + offset), "t");
					nex.client[client].team = atoi(info);
					// Get class
					info = Info_ValueForKey((char *)(nex.cgs_gameState->stringData + offset), "c");
					nex.client[client].cls = atoi(info);
					// Get referee status
					info = Info_ValueForKey((char *)(nex.cgs_gameState->stringData + offset), "ref");
					int ref = atoi(info);
					if (ref) {
						nex.client[client].ref = atoi(info);
						nex.stats.refCount++;
					}
					nex.stats.totalPlayers++;
				} else {
					#ifdef NEX_VEROBSE
						nexUtils.log("CG_GETGAMESTATE: nex.client[%i].infoValid = false");
					#endif
					memset(&nex.client[client], 0, sizeof(nex.client[0]));
				}
			}
			// Get limbotimes
			int offset = nex.cgs_gameState->stringOffsets[CS_SERVERINFO];
			if (offset) {
				nex.cg_redlimbotime = atoi(Info_ValueForKey((char *)(nex.cgs_gameState->stringData + offset), "g_redlimbotime"));
				nex.cg_bluelimbotime = atoi(Info_ValueForKey((char *)(nex.cgs_gameState->stringData + offset), "g_bluelimbotime"));
			}
			// Get cgs.levelStartTime
			offset = nex.cgs_gameState->stringOffsets[CS_LEVEL_START_TIME];
			if (offset)
				nex.cgs_levelStartTime = atoi(nex.cgs_gameState->stringData + offset);
			// Get cgs.aReinfOffset
			offset = nex.cgs_gameState->stringOffsets[CS_REINFSEEDS];
			if (offset)
				nexSdk.CG_ParseReinforcementTimes(nex.cgs_gameState->stringData + offset);
			// Get cgs.gamestate
			offset = nex.cgs_gameState->stringOffsets[CS_WOLFINFO];
			if (offset)
				nex.cgs_gamestate = (gamestate_t) atoi(Info_ValueForKey((char *)(nex.cgs_gameState->stringData + offset), "gamestate"));
			// Get punkbuster state
			offset = nex.cgs_gameState->stringOffsets[CS_SERVERINFO];
			if (offset) {
				int pb = atoi(Info_ValueForKey((char *)(nex.cgs_gameState->stringData + offset), "sv_punkbuster"));
				if (pb && sNex.detected)
						orig_syscall(CG_ERROR, "The bot seems to be detected and this is a punkbuster server, disconnecting for security reasons!");
			}
			break;
		}
		case CG_GETSERVERCOMMAND: {
			const char *cmd = nexSyscall.CG_Argv(0);

			// Sanity check
			if (!cmd || !strlen(cmd))
				break;

			if (!strcmp(cmd, "cs")) {
				// Only update if cgs.gameState already exists
				if (!nex.cgs_gameState)
					break;

				int num = atoi(nexSyscall.CG_Argv(1));
				if (!num)
					break;

				const char *str = nex.cgs_gameState->stringData + nex.cgs_gameState->stringOffsets[num];

				if (!str || !strlen(str))
					break;

				switch (num) {
					case CS_WOLFINFO:
						#ifdef NEX_VERBOSE
							nexUtils.log("Updating cgs_gamestate from %i to %i", nex.cgs_gamestate, atoi(Info_ValueForKey(str, "gamestate")));
						#endif
						nex.cgs_gamestate = (gamestate_t) atoi(Info_ValueForKey(str, "gamestate"));
						break;
					default:
						break;
				}
			} else if (!strcmp(cmd, "spawnserver")) {
				nexHook.hookET(true);
			}
			break;
		}
		default:
			break;
	}

	return result;
}

int nex_UI_syscall(int command, ...) {
	// Get all arguments to send them to the original syscall
	int arg[10];
	va_list arglist;
	va_start(arglist, command);
	int count = 0;
	for (; count < 10; count++)
		arg[count] = va_arg(arglist, int);
	va_end(arglist);

	if (command == UI_CVAR_REGISTER) {
		if (arg[3] & CVAR_CHEAT) {
			#ifdef NEX_VERBOSE
				nexUtils.log("UI: Unlocking cheat cvar: %s", arg[1]);
			#endif

			arg[3] &= ~CVAR_CHEAT;
		}
	}

	if (nex.cg_clientNum != -1) {
		switch (command) {
			case UI_R_DRAWSTRETCHPIC: {
				// Find cursor picture
				qhandle_t hShader = (qhandle_t) arg[8];
				// Get game mouse position
				if (hShader == nex.media.cursorIcon && nex.cg_clientNum != -1) {
					nex.mouse[0] = *(float *) &arg[0] / nex.cgs_screenXScale;
					nex.mouse[1] = *(float *) &arg[1] / nex.cgs_screenYScale;
				}
				break;
			}
			default:
				break;
		}
	}

	switch (command) {
		case UI_PC_LOAD_SOURCE: {
			char *name = (char *)arg[0];
	
			// If invalid name
			if (!name || !strlen(name))
				break;
	
			if (!strcmp(name, "ui/ingame_messagemode.menu"))
				arg[0] = (int) "ui/nex_msgpopup.menu";
	
			break;
		}
		default:
			break;
	}

	return orig_syscall(command, arg[0], arg[1], arg[2], arg[3], arg[4], arg[5], arg[6], arg[7], arg[8], arg[9]);
}

void __cdecl nex_CG_DrawActiveFrame(int serverTime, int stereoView, qboolean demoPlayback) {
	nex.cg_frametime = serverTime - nex.cg_time;
	nex.cg_time = serverTime;
	nexEngine.prev_CG_DrawActiveFrame();
	nex.orig.CG_DrawActiveFrame(serverTime, stereoView, demoPlayback);
	nexEngine.post_CG_DrawActiveFrame();
}

void __cdecl nex_CG_Init(int serverMessageNum, int serverCommandSequence, int clientNum) {
	nex.cg_clientNum = clientNum;
	nexEngine.prev_CG_Init();
	nex.orig.CG_Init(serverMessageNum, serverCommandSequence, clientNum);
	nexEngine.post_CG_Init();
}

void __cdecl nex_CG_FinishWeaponChange(int lastweap, int newweap) {
	bool callOriginal = false;
	
	if (!((lastweap == WP_K43_SCOPE && newweap == WP_K43) || (lastweap == WP_GARAND_SCOPE && newweap == WP_GARAND) || (lastweap == WP_FG42SCOPE && newweap == WP_FG42)))
		callOriginal = true;

	int weapon = lastweap;
	switch (weapon) {
		case WP_K43_SCOPE:
			weapon = WP_K43;
			break;
		case WP_GARAND_SCOPE:
			weapon = WP_GARAND;
			break;
		case WP_FG42SCOPE:
			weapon = WP_FG42;
			break;
		default:
			break;
	}

	if (nex.cg_snap->ps.ammoclip[weapon] == 0)
		callOriginal = true;

	if (nexSyscall.isKeyActionDown("+reload") || nexSyscall.isKeyActionDown("weapalt"))
		callOriginal = true;

	if (callOriginal)
		nex.orig.CG_FinishWeaponChange(lastweap, newweap);
}

void __cdecl nex_CG_DamageFeedback(int, int, int) {}

void __cdecl nex_CG_EntityEvent(centity_t *cent, vec3_t position) {
	entityState_t *es = &cent->currentState;;
	int event = es->event & ~EV_EVENT_BITS;

	// this 99.999% won't happen, but something is crashing the bot
	// and i wanna eliminate some things
	if (!nex.cg_snap)
		nex.orig.CG_EntityEvent(cent, position);

	switch (event) {
		case EV_OBITUARY:
			// Don't count kills in warmup.
			if (nex.cgs_gamestate != GS_PLAYING)
				break;
			// Report if it doesen't work on some mods
			int target, attacker;
			if (nex.mod->type == MOD_ETPRO) {
				target = es->time;
				attacker = es->time2;
			} else {
				target = es->otherEntityNum;
				attacker = es->otherEntityNum2;
			}
			// If we do a selfkill or someone kills us, reset killing spree
			if (target == nex.cg_clientNum) {
				nex.stats.killCountNoDeath = 0;
				nex.stats.killSpreeCount = 0;
				break;
			// Increase killcount only if we're the attacker and if we killed a player from the opposite team
			} else if (attacker == nex.cg_clientNum && nex.client[target].team != nex.client[nex.cg_clientNum].team) {
				nex.stats.lastVictim = target;
				nex.stats.killCount++;
				nex.stats.killCountNoDeath++;
				// If not a killing spree
				if ((nex.cg_time - nex.stats.lastKillTime) > (NEX_SPREE_TIME)) {
					nex.stats.firstKillSpreeTime = nex.cg_time;
					nex.stats.killSpreeCount = 1;
				} else {
					nex.stats.killSpreeCount++;
				}
				nex.stats.lastKillTime = nex.cg_time;
/*				if ((nex.stats.killSpreeCount > 1) && nexValue[VAR_SPREESOUNDS])
					nexSpree.playSpreeSound();*/
				if (nexValue[VAR_KILLSPAM])
					nexSpree.killSpam();
			}
			break;
		default:
			break;
	}
	__asm pushad
	nex.orig.CG_EntityEvent(cent, position);
	__asm popad
}

const char* __cdecl nex_CG_ConfigString(int index) {
	if(index == CS_WOLFINFO && !nex.hHideFromAC) {
		#ifdef NEX_VERBOSE
			nexUtils.log("CG_ConfigString() called - bypassing etpro AC");
		#endif
		nexHook.hookCG(false);
		nex.hHideFromAC = CTHREAD(HideFromAC);
	}

	return nex.orig.CG_ConfigString(index);
}

void nex_FS_PureServerSetLoadedPaks(const char *pakSums, const char *pakNames) {
	#ifdef NEX_VERBOSE
		nexUtils.log("FS_PureServerSetLoadedPaks() called");
	#endif
	static char fakePakSums[BIG_INFO_STRING];
	static char fakePakNames[BIG_INFO_STRING];

	strncpy(goodChecksumsPak, pakSums, sizeof(goodChecksumsPak));
	pack_t *nexPack = nexHook.getPack(sNex.pk3Name);

	// If not pure server
	if (!strlen(pakSums) && !strlen(pakNames)) {
		orig_FS_PureServerSetLoadedPaks(pakSums, pakNames);
	// Add nex pk3
	} else if (nexPack) {
		#ifdef NEX_VERBOSE
			nexUtils.log("[PK3] pakNames: %s", pakNames);
		#endif
		sprintf_s(fakePakSums, sizeof(fakePakSums), "%s%i ", pakSums, nexPack->checksum);
		sprintf_s(fakePakNames, sizeof(fakePakNames), "%s %s/%s", pakNames, nexPack->pakGamename, nexPack->pakBasename);
		orig_FS_PureServerSetLoadedPaks(fakePakSums, fakePakNames);
		#ifdef NEX_VERBOSE
			nexUtils.log("[PK3] fakePakNames: %s", fakePakNames);
		#endif
	} else {
		#ifdef NEX_LOGGING
			nexUtils.log("ERROR PK3: '%s' not found.", sNex.pk3Name);
		#endif
		orig_FS_PureServerSetLoadedPaks(pakSums, pakNames);
	}
}

void CNexHook::unreferenceBadPk3s() {
	// Not pure server
	if (!strlen(goodChecksumsPak))
		return;
	
	// Parse all paks
	searchpath_t *browse = *(searchpath_t **)sNex.ET->fs_searchpaths;
	for (; browse; browse = browse->next) {
		if (!browse->pack)
			continue;

		// Parse all checksums for find this one
		bool purePak = false;
		char *checksum = goodChecksumsPak;
		char *nextChecksum;
		while ((nextChecksum = strchr(checksum, ' '))) {
			char *sum = nexUtils.strndup(checksum, nextChecksum - checksum);

			// Check if in pure list
			if (browse->pack->checksum == atoi(sum))
				purePak = true;

			checksum = nextChecksum + 1;
			free(sum);
		};

		// If not in server pk3 list and referenced
		if (!purePak && browse->pack->referenced && strstr(browse->pack->pakFilename, sNex.pk3Name)) {
			browse->pack->referenced = 0;
			#ifdef NEX_VERBOSE
				nexUtils.log("PK3: unreferencing invalid pak: %s\n", browse->pack->pakFilename);
			#endif
		}
	}
}

// need to hook this shyt because of pk3 unlokker
xcommand_t orig_cc_vid_restart;
xcommand_t orig_cc_connect;
xcommand_t orig_cc_quit;

void nex_cc_connect (void) {
	nexHook.hookET(true);
	orig_cc_connect();
}

void nex_cc_vid_restart (void) {
	nexHook.hookET(true);
	orig_cc_vid_restart();
}

void nex_cc_quit (void) {
	nexNetwork.saveIrcNickToConfig();
	orig_cc_quit();
}

void nex_Cmd_AddCommand(const char *cmd_name, xcommand_t function) {
	if (!strcmp(cmd_name, "vid_restart")) {
		orig_cc_vid_restart = function;
		function = nex_cc_vid_restart;
	} else if (!strcmp(cmd_name, "connect")) {
		orig_cc_connect = function;
		function = nex_cc_connect;
	} else if (!strcmp(cmd_name, "quit")) {
		orig_cc_quit = function;
		function = nex_cc_quit;
	}

	orig_Cmd_AddCommand(cmd_name, function);
}

cvar_t *__cdecl nex_Cvar_Get(const char *var_name, const char *var_value, int flags) {
	if (flags & CVAR_CHEAT) {
		#ifdef NEX_VERBOSE
			nexUtils.log("Cvar_Get: Unlocking cheat cvar %s", var_name);
		#endif

		flags &= ~CVAR_CHEAT;
	}

	return orig_Cvar_Get(var_name, var_value, flags);
}

void __cdecl nex_CG_Trace(trace_t *result, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int skipNumber, int mask) {
	nex.orig.CG_Trace(result, start, mins, maxs, end, skipNumber, mask);
	// make TCE think a player entity is visible
	if (skipNumber == nex.cg_clientNum)
		result->fraction = 1;
}
