// GPL License - see http://opensource.org/licenses/gpl-license.php
// Copyright 2006 *nixCoders team - don't forget to credit us

#pragma once // main header file

// disable some warnings
#define _CRT_SECURE_NO_DEPRECATE	// function or variable may be unsafe
#pragma warning(disable: 4800)		// forcing int to bool
#pragma warning(disable: 4312)		// const DWORD to BYTE * of greater size
#pragma warning(disable: 4311)		// pointer truncation
#pragma warning(disable: 4789)		// destination of memory copy is too small - dunno why this happens?!
#pragma warning(disable: 4267)		// size_t to int
#pragma warning(disable: 4700)		// uninitialized local variable

// version info
#define NEX_VERSION	"0.4"	// version string
#define NEX_EDITION	"Leff"	// edition name

// default values
#define NEX_PREFIX	"zh"				// cmd prefix
#define NEX_CONFIG	"nex_settings.src"	// config file
#define NEX_INI		"nexus_config.ini"	// configure some stuff here insteat of in 'real' cfg file
#define NEX_PK3		"nexuz.pk3"			// pk3 file (c:\path\to\et\etmain\nexus.pk3)
#define NEX_EG		"OpensourcePowa"	// custom etpro guid
#define NEX_ES		true				// enable spoofing by default
#define NEX_KS		"^0[^3NEXUS^0] ^nowned ^1>^7[n]^1< ^0| ^nSpree: ^1[c] ^0| ^nTotal: ^1[t]"
#define NEX_MSGC	"^n"				// message color used for console printing and echoing
#define NEX_USEDET	true				// check for detection status
#define NEX_USEIRC	true				// enable irc
#define NEX_USESPC	true				// get list of ppl that are specing us
#define NEX_USEVER	true				// check if we're up2date

// logging - comment/uncomment to toggle logging type
// NEX_LOGGING: log basic stuff, NEX_VERBOSE: log a lot of stuff, huge file during gameplay
// #define NEX_VERBOSE
// #define NEX_LOGGING

#ifdef NEX_VERBOSE
#ifndef NEX_LOGGING
#define NEX_LOGGING
#endif
#endif

#ifdef NEX_LOGGING
	#define NEX_LOGFILE	"nexus.log"			// log file (c:\path\to\nexus\nexus.log)
#endif

// include 'main' headers
#include <fstream>
#include <windows.h>
#include <detours.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <winsock.h>
#include <stdlib.h>

// import some libs
#pragma comment(lib, "wsock32.lib")			// sockets

// sdk
#include "sdk/src/cgame/cg_local.h"
#include "sdk/src/ui/ui_public.h"
#define PASSFLOAT(x) (*(int*)&x)

// include some tools
#include "tools/crc32.h"		// http://www.csbruce.com/~csbruce/software/crc32.c
#include "tools/IniReader.h"	// http://www.codeproject.com/useritems/IniReader.asp
#include "tools/IniWriter.h"	// url same as above, but the code needs to be fixed!

// our inkludes
#include "sdk.h"
#include "menu.h"
#include "network.h"
#include "types.h"
#include "utils.h"
#include "hook.h"
#include "syscalls.h"
#include "engine.h"
#include "drawtools.h"
#include "visuals.h"
#include "g_functions.h"
#include "filters.h"
#include "aimbot.h"
#include "spycams.h"
#include "sprees.h"
#include "offsets.h"

// structs
extern sNex_t sNex;
extern nex_t nex;
