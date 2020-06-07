// GPL License - see http://opensource.org/licenses/gpl-license.php
// Copyright 2006 *nixCoders team - don't forget to credit us

#include "nexus.h"

CNexVisuals nexVisuals;

vec4_t gameBackColor	= { 0.16f,	0.2f,	0.17f,	0.9f };
vec4_t gameBorderColor	= { 0.5f,	0.5f,	0.5f,	0.5f };
vec4_t gameFontColor	= { 0.625f,	0.625f,	0.6f,	1.0f };

static nexPlayerColor_t playersColorsDef[] = {
	// Type 1
	{
		{ GFX_BLUE,		colorBlue },
		{ GFX_CYAN,		colorCyan },
		{ GFX_RED,		colorRed },
		{ GFX_TEAL,		colorMagenta }
	},
	// Type 2
	{
		{ GFX_GREEN,	colorGreen },
		{ GFX_YELLOW,	colorYellow },
		{ GFX_TEAL,		colorMagenta },
		{ GFX_CYAN,		colorCyan }
	},
	// Type 3
	{
		{ GFX_NONE,		colorWhite },
		{ GFX_NONE,		colorMdGrey },
		{ GFX_RED,		colorRed },
		{ GFX_TEAL,		colorMagenta }
	}
};

void CNexVisuals::drawAdvert() {
	#define ADVERT_SIZE 0.42f
	#define ADVERT_Y 20

	static char *adv = va("^0 [^3*C^0] ^nnexus ^7v%s ^0- ^3*^nnixCoders ", NEX_VERSION);

	static vec4_t bgColor = { 0.28f, 0.25f, 0.19f, 0.75f };
	static int width = nexDraw.sizeTextWidth(ADVERT_SIZE, adv, &nex.media.mainFont);
	static int posX = 320 - (width / 2);

	nexDraw.drawTextRect(posX, ADVERT_Y, ADVERT_SIZE, adv, colorWhite, qfalse, qtrue, qtrue, qtrue, bgColor, colorWhite, &nex.media.mainFont);
}

void CNexVisuals::drawRefCount() {
	static vec4_t refBackground	= { 0.16f,	0.2f,	0.17f,	0.8f };
	static vec4_t refBorder		= { 0.5f,	0.5f,	0.5f,	0.5f };
	
	char str[24];
    sprintf_s(str, sizeof(str), "^nRefs: %i", nex.stats.refCount);
    int w = nexDraw.sizeTextWidth(0.19f, str, &nex.media.mainFont);
    int y = 137;
    int x = 634 - w;
    nexDraw.drawFillRect(x - 2, y, w + 5, 12 + 2, refBackground);
    nexDraw.drawRect(x - 2, y, w + 5, 12 + 2, refBorder, 1);
    nexDraw.drawText(x, y + 3, 0.19f, str, colorRed, qfalse, qfalse, &nex.media.mainFont);
}

void CNexVisuals::drawSpawnTimer() {
	int spawntime = nexEngine.getSpawntime(true);
	// No spanwtimer if not in a team
	if (spawntime == -1)
		return;

	static vec4_t timerBackground	= { 0.16f,	0.2f,	0.17f,	0.8f };
	static vec4_t timerBorder		= { 0.5f,	0.5f,	0.5f,	0.5f };
	
	char str[6];
    sprintf_s(str, sizeof(str), "%i", spawntime);
    int w = nexDraw.sizeTextWidth(0.19f, str, &nex.media.mainFont);
    int y = 136 - 14;
    int x = 634 - w;
    nexDraw.drawFillRect(x - 2, y, w + 5, 12 + 2, timerBackground);
    nexDraw.drawRect(x - 2, y, w + 5, 12 + 2, timerBorder, 1);
    nexDraw.drawText(x, y + 3, 0.19f, str, colorRed, qtrue, qfalse, &nex.media.mainFont);
}

void CNexVisuals::addChams(refEntity_t* refEnt, bool visible) {
	// Specials settings on some chams
	if (nexValue[VAR_CHAMS] == CHAM_GLOW) {
		nexSyscall.R_AddRefEntityToScene(refEnt);
	} else if (nexValue[VAR_CHAMS] == CHAM_QUAKEC) {
	    refEntity_t original = *refEnt;
	    original.customShader = nex.media.whiteFlatShader;
		
		if ((original.entityNum < MAX_CLIENTS) && !IS_PLAYER_ENEMY(original.entityNum)) {
			original.shaderRGBA[2] = 255;
			original.shaderRGBA[3] = 200;           
		} else {
			// treat all non-players as enemy entities.
			original.shaderRGBA[0] = 255;
			original.shaderRGBA[3] = 200;           
		}
	    
	    nexSyscall.R_AddRefEntityToScene(&original);
	}
		
	if (!IS_PLAYER_ENEMY(refEnt->entityNum)) {
		if (visible)
			refEnt->customShader = nex.media.friendVisibleShader;
		else
			refEnt->customShader = nex.media.friendNotVisibleShader;
	} else {
		// treat all non-players as enemy entities.
		if (visible) 
			refEnt->customShader = nex.media.enemyVisibleShader;
		else
			refEnt->customShader = nex.media.enemyNotVisibleShader;
	}
}

void CNexVisuals::addEspColors(int entityNum, bool visible) {
	if (!IS_PLAYER_ENEMY(entityNum)) {
		if (visible)
			nex.entities[entityNum].espColor = (float *)&nex.media.friendVisibleColor;
		else
			nex.entities[entityNum].espColor = (float *)&nex.media.friendNotVisibleColor;
	} else {
		if (visible)
			nex.entities[entityNum].espColor = (float *)&nex.media.enemyVisibleColor;
		else
			nex.entities[entityNum].espColor = (float *)&nex.media.enemyNotVisibleColor;
	}
}

void CNexVisuals::setColors() {
	// Sanity check
	if ((nexValue[VAR_COLORS] < 0) || (nexValue[VAR_COLORS] >= (sizeof(playersColorsDef) / sizeof(nexPlayerColor_t)))) {
		#ifdef NEX_LOGGING
			nexUtils.log("WARNING [visual]: bad color %i. resetting to default", nexValue[VAR_COLORS]);
		#endif
		nexValue[VAR_COLORS] = nexVars[VAR_COLORS].defaultValue;
	}

	// Sanity check
	if ((nexValue[VAR_CHAMS] < 0) || (nexValue[VAR_CHAMS] >= CHAMS_TOTAL)) {
		#ifdef NEX_LOGGING
			nexUtils.log("WARNING [visual]: bad chams %i. resetting to default", nexValue[VAR_COLORS]);
		#endif
		nexValue[VAR_CHAMS] = nexVars[VAR_CHAMS].defaultValue;
	}

	qhandle_t chams = nexValue[VAR_CHAMS];

	// Special settings on some chams
	if (chams == CHAM_GLOWONLY)
		chams = CHAM_GLOW;
	
	nexPlayerColor_t *playerColor = &playersColorsDef[(int)nexValue[VAR_COLORS]];

	// Friend visible
	nex.media.friendVisibleShader = nex.media.shaders[playerColor->friendVisible.chams][chams];
	VectorCopy(playerColor->friendVisible.esp, nex.media.friendVisibleColor);

	// Enemy visible
	nex.media.enemyVisibleShader = nex.media.shaders[playerColor->enemyVisible.chams][chams];
	VectorCopy(playerColor->enemyVisible.esp, nex.media.enemyVisibleColor);

	// Specials settings on some chams
	if (chams == CHAM_QUAKEC)
		chams = CHAM_QUAKE;

	// Friend not visible
	nex.media.friendNotVisibleShader = nex.media.shaders[playerColor->friendNotVisible.chams][chams];
	VectorCopy(playerColor->friendNotVisible.esp, nex.media.friendNotVisibleColor);

	// Enemy not visible
	nex.media.enemyNotVisibleShader = nex.media.shaders[playerColor->enemyNotVisible.chams][chams];
	VectorCopy(playerColor->enemyNotVisible.esp, nex.media.enemyNotVisibleColor);
}

bool CNexVisuals::isRadar(int player, int *screen, float scale) {
	vec3_t vec,rot;
	float distance;

	if(nex.cg_entities[player].currentState->eFlags & EF_DEAD)
		return false;

	if (nex.client[player].team == TEAM_SPECTATOR)
		return false;

	if (player == nex.cg_snap->ps.clientNum)
		return false;

	if (!nex.entities[player].distance)
		return false;

	VectorSubtract(nex.entities[player].origin, nex.cg_refdef.vieworg, vec);
	vec[2] = 0.0f;
	distance = VectorLength(vec) * 0.025;
	vectoangles(vec, rot);
	rot[1] = AngleNormalize180(rot[1] - nex.cg_refdefViewAngles[1]);
	AngleVectors(rot, vec, NULL, NULL);
	VectorScale(vec, distance, vec);

    if(vec[0] > scale)
       VectorScale(vec, scale / vec[0], vec);
    else if(vec[0] < -scale)
       VectorScale(vec, -scale / vec[0], vec);
    if(vec[1]>100.0f)
       VectorScale(vec, scale / vec[1], vec);
    else if(vec[1] < -scale)
       VectorScale(vec, -scale / vec[1], vec);

    screen[0] = ((int)-vec[1] + (640-scale)) - 2;
    screen[1] = ((int)-vec[0] + scale) - 2;

	return true;
}

void CNexVisuals::drawRadar(int scale) {
    static vec4_t radarcolor = { 0.2f, 0.9f, 0.2f, 0.25f };
	static vec4_t colorBlackR = { 0.7f, 0.7f, 0.7f, 0.50f };

	// some cheks so radar isn't way too big or too small
	if (scale > 120) scale = 120;
	else if (scale < 50) scale = 50;

	nexDraw.drawFillRect(640-scale, 0,scale, scale, radarcolor);
	nexDraw.drawFillRect(640-((scale/4)*3), 0, 1, scale, colorBlackR);
	nexDraw.drawFillRect(640-(scale/4), 0, 1, scale, colorBlackR);
	nexDraw.drawFillRect(640-scale, scale/4, scale, 1, colorBlackR);
	nexDraw.drawFillRect(640-scale, (scale/4)*3, scale, 1, colorBlackR);
	nexDraw.drawFillRect(640-(scale/2), 0, 1, scale, colorBlack);
	nexDraw.drawFillRect(640-scale, scale/2, scale, 1, colorBlack);

	int screen[2];
	int player = 0;
	for (; player < MAX_CLIENTS; player++) {
		if (nex.client[player].infoValid && isRadar(player, screen, (float) scale/2)) {
			nexEntity_t *entity = &nex.entities[player];
			// Sanity check
			if (!entity->espColor) {
				addEspColors(player, entity->isVisible);
				if (!entity->espColor) {
					// TODO: find out why ent 0 doesen't get set
					#ifdef NEX_LOGGING
						nexUtils.log("ERROR: RADAR, can't find color for entity %i", player);
					#endif
					continue;
				}
			}

			vec4_t color;
			VectorCopy(entity->espColor, color);
			color[3] = 1.0f;

			if (screen[0] < 640-scale+3)
				screen[0] = 640-scale+3;
			if (screen[0] > 637)
				screen[0] = 637;
			if (screen[1] > scale-5)
				screen[1] = scale-5;

			nexDraw.drawFillRect(screen[0], screen[1], 3, 3, color);
	    }
	}
}

void CNexVisuals::drawEspEntities() {
	// it should never get called if a snapshot isn't retreived, but adding check to be sure
	if (!nex.cg_snap)
		return;

	int entityNum;

	// Player esp
	for (entityNum = 0; entityNum < MAX_CLIENTS; entityNum++) {
		nexEntity_t *entity = &nex.entities[entityNum];
		
		if (!nex.client[entityNum].infoValid || !entity->isInScreen || (nex.cg_snap->ps.clientNum == entityNum))
			continue;

		if (IS_PLAYER_DEAD(entityNum))
			continue;

		// Sanity check
		if (!entity->espColor) {
			addEspColors(entityNum, entity->isVisible);
			if (!entity->espColor) {
				// TODO: find out why ent 0 doesen't get set
				#ifdef NEX_LOGGING
					nexUtils.log("ERROR: ESP, can't find color for entity %i", entityNum);
				#endif
				continue;
			}
		}

		// Define fade color for both name and icon
		float fadeColor = 1.0f;
		if (nexValue[VAR_ESP_FADE]) {
			fadeColor = 800.0f / entity->distance;
			fadeColor = max(fadeColor, 0.2f);	// Max fade
			fadeColor = min(fadeColor, 1.0f);	// Min fade
		}

		static float fontScale = 0.19f;
		int y = entity->screenY - nexDraw.sizeTextHeight(fontScale, &nex.media.mainFont);

		// Name esp
		if (nexValue[VAR_ESP_NAME]) {
			// Position
			int x = entity->screenX - (nexDraw.sizeTextWidth(fontScale, nex.client[entityNum].name, &nex.media.mainFont) / 2);

			// Define name color	
			vec4_t nameColor;
			Vector4Copy(colorWhite, nameColor);
			nameColor[3] = fadeColor;

			nexDraw.drawText(x, y, fontScale, nex.client[entityNum].name, nameColor, qfalse, qtrue, &nex.media.mainFont);
		}

		// Weapon icon esp
		if (nexValue[VAR_ESP_CW] && nex.mod->type != MOD_TCE) {
			static float iconSize = 15.0f;

			// Position
			int x = entity->screenX - (iconSize * 3) / 2;
			y -= iconSize + 4;

			// Icon color
			vec4_t iconColor;
			VectorCopy(entity->espColor, iconColor);
			iconColor[3] = fadeColor;

			nexSyscall.R_SetColor(iconColor);
		    // Class
			nexDraw.drawPic(x, y, iconSize, iconSize, nex.media.classIcons[nex.client[entityNum].cls]);
			// Weapon
			nexDraw.drawPic(x + iconSize, y, iconSize * 2, iconSize, nex.media.weaponIcons[nex.cg_entities[entityNum].currentState->weapon]);
		    
			nexSyscall.R_SetColor(NULL);		
		}
	}
	
	// Missile esp
	for (entityNum = MAX_CLIENTS; entityNum < MAX_GENTITIES && nexValue[VAR_ESP_MISSILE]; entityNum++) {

		nexEntity_t *entity = &nex.entities[entityNum];

		if (!*nex.cg_entities[entityNum].currentValid || !entity->isInScreen || !IS_MISSILE(entityNum))
			continue;

		// Landmine esp
		if(IS_LANDMINE(entityNum) && IS_MISSILE_ARMED(entityNum)) {
			nexDraw.drawText(entity->screenX - 4 , entity->screenY, 0.19f, "L", colorYellow, qfalse, qtrue, &nex.media.mainFont);
		// Dynamite esp
		} else if(IS_DYNAMITE(entityNum)) {
			char timeStr[8];
			int max, time;

			if (nex.mod->type == MOD_TCE)
				max = 45;
			else
				max = 30;
		
			time = max - ((nex.cg_time - nex.cg_entities[entityNum].currentState->effect1Time) / 1000);
			
			if (time > 0) {
				sprintf(timeStr, "D:%i", time);
				nexDraw.drawText(entity->screenX - 16, entity->screenY, 0.19f, timeStr, colorYellow,qfalse, qtrue, &nex.media.mainFont);
			} else
			nexDraw.drawText(entity->screenX - 4, entity->screenY, 0.19f, "D", colorYellow, qfalse, qtrue, &nex.media.mainFont);
		// Grenade esp
		} else if (IS_GRENADE(entityNum)) {
			nexDraw.drawText(entity->screenX - 4, entity->screenY, 0.19f, "G", colorYellow, qfalse, qtrue, &nex.media.mainFont);
		// Satchel esp
		} else if (IS_SATCHEL(entityNum)) {
			nexDraw.drawText(entity->screenX - 4, entity->screenY, 0.19f, "T", colorYellow, qfalse, qtrue, &nex.media.mainFont);
		// Panzer esp
		} else if (IS_PANZER(entityNum)) {
			nexDraw.drawText(entity->screenX - 4, entity->screenY, 0.19f, "P", colorYellow, qfalse, qtrue, &nex.media.mainFont);
		// Mortar esp
		} else if (IS_MORTAR(entityNum)) {
			nexDraw.drawText(entity->screenX - 4, entity->screenY, 0.19f, "M", colorYellow, qfalse, qtrue, &nex.media.mainFont);
		// smoke esp
		} else if (IS_SMOKE(entityNum)) {
			nexDraw.drawText(entity->screenX - 4, entity->screenY, 0.19f, "S", colorYellow, qfalse, qtrue, &nex.media.mainFont);
		}
	}
}

void CNexVisuals::drawIrcChat() {
	static vec4_t	bdColor = { 1.f, 1.f, 1.f, 1.f };
	static vec4_t	txColor = { 1.f, 1.f, 1.f, 1.f };
	static vec4_t	bgColor = { 0.28f, 0.25f, 0.19f, 0.75f };

	static bool show = false;
	if (nex.lastIrcCheck < nex.cg_time - IRC_CHAT_TIME) {
		if (nexSyscall.CG_Key_IsDown(IRC_CHAT_KEY)) {
			show = !show;
			nex.lastIrcCheck = nex.cg_time;
		}
	}

	if (show) {
		static int lineHeight = nexDraw.sizeTextHeight(IRC_CHAT_TEXT_SIZE, &nex.media.mainFont) + IRC_CHAT_SPACE;

		nexDraw.drawFillRect(IRC_CHAT_X, IRC_CHAT_Y, IRC_CHAT_W, IRC_CHAT_H, bgColor);
		nexDraw.drawRect(IRC_CHAT_X, IRC_CHAT_Y, IRC_CHAT_W, IRC_CHAT_H, bdColor, IRC_CHAT_BORDER_SIZE);

		int count = 0;
		for (; count < IRC_MAX_MESSAGES; count++) {
			if (!ircMessages[count].msg || !strlen(ircMessages[count].msg))
				break;

			int y = IRC_CHAT_Y + IRC_CHAT_H - IRC_CHAT_SPACE - (count + 1) * lineHeight;

			if (y < IRC_CHAT_Y)
				break;

			nexDraw.drawText(IRC_CHAT_X + IRC_CHAT_SPACE, y, IRC_CHAT_TEXT_SIZE, ircMessages[count].msg, txColor, qfalse, qtrue, &nex.media.mainFont);
		}
	}
}

void CNexVisuals::drawSpectators() {
	#define SPECBOX_X 10
	#define SPECBOX_Y 135
	#define SPECBOX_FONT_SIZE 0.19f
	#define SPECBOX_SPACE 5
	
	static vec4_t bdColor = { 0.8f, 0.8f, 0.8f, 1.f };
	static vec4_t bgColor = { 0.28f, 0.25f, 0.19f, 0.75f };

	// If no spectators
	if (!nex.stats.specCount)
		return;

	char specs[15];
	if (nex.stats.specCount == 1)
		sprintf_s(specs, sizeof(specs), "^3%i spectator:", nex.stats.specCount);
	else
		sprintf_s(specs, sizeof(specs), "^3%i spectators:", nex.stats.specCount);

	// Get max width
	int max = 0;
	int count = 0;
	for (; count < nex.stats.specCount; count++) {
		int w = nexDraw.sizeTextWidth(SPECBOX_FONT_SIZE, nex.stats.specNames[count], &nex.media.mainFont);
		if (w > max)
			max = w;
	}
	if (nexDraw.sizeTextWidth(SPECBOX_FONT_SIZE, specs, &nex.media.mainFont) > max)
		max = nexDraw.sizeTextWidth(SPECBOX_FONT_SIZE, specs, &nex.media.mainFont);

	int width = max + (2 * SPECBOX_SPACE);
	int lineHeight = nexDraw.sizeTextHeight(SPECBOX_FONT_SIZE, &nex.media.mainFont) + SPECBOX_SPACE;
	int height = ((lineHeight * (nex.stats.specCount + 1)) + SPECBOX_SPACE);
	int y = SPECBOX_Y + SPECBOX_SPACE;

	nexDraw.drawFillRect(SPECBOX_X, SPECBOX_Y, width, height, bgColor);
	nexDraw.drawRect(SPECBOX_X, SPECBOX_Y, width, height, bdColor, 1);
	nexDraw.drawText(SPECBOX_X + SPECBOX_SPACE, y, SPECBOX_FONT_SIZE, specs, colorRed, qfalse, qtrue, &nex.media.mainFont);
	y += lineHeight;
	
	for (count = 0; count < nex.stats.specCount; count++) {
		nexDraw.drawText(SPECBOX_X + SPECBOX_SPACE, y, SPECBOX_FONT_SIZE, nex.stats.specNames[count], colorWhite, qfalse, qtrue, &nex.media.mainFont);
		y += lineHeight;
	}
}

void CNexVisuals::drawVersionWarning() {
	static vec4_t timerBackground	= { 0.16f,	0.2f,	0.17f,	0.8f };
	static vec4_t timerBorder		= { 0.5f,	0.5f,	0.5f,	0.5f };

	char *str = sNex.updated ? "^2UPDATED" : "^8OUTDATED";

	int w = nexDraw.sizeTextWidth(0.19f, str, &nex.media.mainFont);
    int y = 107;
    int x = 634 - w;
    nexDraw.drawFillRect(x - 2, y, w + 5, 12 + 2, timerBackground);
    nexDraw.drawRect(x - 2, y, w + 5, 12 + 2, timerBorder, 1);
    nexDraw.drawText(x, y + 3, 0.19f, str, colorRed, qfalse, qfalse, &nex.media.mainFont);
}

