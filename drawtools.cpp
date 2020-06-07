// GPL License - see http://opensource.org/licenses/gpl-license.php
// Copyright 2006 *nixCoders team - don't forget to credit us

#include "nexus.h"

CNexDraw nexDraw;

int CNexDraw::CG_Text_Width_Ext(const char *text, float scale, int limit, fontInfo_t* font) {
	int count, len;
	glyphInfo_t *glyph;
	const char *s = text;
	float out, useScale = scale * font->glyphScale;
	
	out = 0;
	if( text ) {
		len = (int) strlen( text );
		if (limit > 0 && len > limit) {
			len = limit;
		}
		count = 0;
		while (s && *s && count < len) {
			if ( Q_IsColorString(s) ) {
				s += 2;
				continue;
			} else {
				glyph = &font->glyphs[(unsigned char)*s];
				out += glyph->xSkip;
				s++;
				count++;
			}
		}
	}

	return out * useScale;
}

void CNexDraw::CG_Text_PaintChar_Ext(float x, float y, float w, float h, float scalex, float scaley, float s, float t, float s2, float t2, qhandle_t hShader) {
	w *= scalex;
	h *= scaley;
	adjustFrom640(&x, &y, &w, &h);
	nexSyscall.R_DrawStretchPic(x, y, w, h, s, t, s2, t2, hShader);
}

void CNexDraw::CG_Text_Paint_Ext(float x, float y, float scalex, float scaley, vec4_t color, const char *text, float adjust, int limit, int style, fontInfo_t* font) {
	int len, count;
	vec4_t newColor;
	glyphInfo_t *glyph;

	scalex *= font->glyphScale;
	scaley *= font->glyphScale;

	if (text) {
		const char *s = text;
		nexSyscall.R_SetColor( color );
		memcpy(&newColor[0], &color[0], sizeof(vec4_t));
		len = (int) strlen(text);
		if (limit > 0 && len > limit) {
			len = limit;
		}
		count = 0;
		while (s && *s && count < len) {
			glyph = &font->glyphs[(unsigned char)*s];
			if ( Q_IsColorString( s ) ) {
				if( *(s+1) == COLOR_NULL ) {
					memcpy( newColor, color, sizeof(newColor) );
				} else {
					memcpy( newColor, g_color_table[ColorIndex(*(s+1))], sizeof( newColor ) );
					newColor[3] = color[3];
				}
				nexSyscall.R_SetColor(newColor);
				s += 2;
				continue;
			} else {
				float yadj = scaley * glyph->top;
				yadj -= (font->glyphs['['].top + 3) * scaley;	// eth: Add y correction
				if (style == ITEM_TEXTSTYLE_SHADOWED || style == ITEM_TEXTSTYLE_SHADOWEDMORE) {
					int ofs = style == ITEM_TEXTSTYLE_SHADOWED ? 1 : 2;
					colorBlack[3] = newColor[3];
					nexSyscall.R_SetColor( colorBlack );
					CG_Text_PaintChar_Ext(x + (glyph->pitch * scalex) + ofs, y - yadj + ofs, glyph->imageWidth, glyph->imageHeight, scalex, scaley, glyph->s, glyph->t, glyph->s2, glyph->t2, glyph->glyph);
					colorBlack[3] = 1.0;
					nexSyscall.R_SetColor( newColor );
				}
				CG_Text_PaintChar_Ext(x + (glyph->pitch * scalex), y - yadj, glyph->imageWidth, glyph->imageHeight, scalex, scaley, glyph->s, glyph->t, glyph->s2, glyph->t2, glyph->glyph);
				x += (glyph->xSkip * scalex) + adjust;
				s++;
				count++;
			}
		}
		nexSyscall.R_SetColor( NULL );
	}

}

void CNexDraw::adjustFrom640(float *x, float *y, float *w, float *h) {
	*x *= nex.cgs_screenXScale;
	*y *= nex.cgs_screenYScale;
	*w *= nex.cgs_screenXScale;
	*h *= nex.cgs_screenYScale;
}

void CNexDraw::adjustTo640(float *x, float *y, float *w, float *h) {
	*x /= nex.cgs_screenXScale;
	*y /= nex.cgs_screenYScale;
	*w /= nex.cgs_screenXScale;
	*h /= nex.cgs_screenYScale;
}

void CNexDraw::drawRect(float x, float y, float width, float height, vec4_t color, int borderSize) {
	nexSyscall.R_SetColor(color);

	adjustFrom640(&x, &y, &width, &height);
	nexSyscall.R_DrawStretchPic(x, y, borderSize, height, 0, 0, 0, 0, nex.media.whiteShader);
	nexSyscall.R_DrawStretchPic(x + width - borderSize, y, borderSize, height, 0, 0, 0, 0, nex.media.whiteShader);
	nexSyscall.R_DrawStretchPic(x, y, width, borderSize, 0, 0, 0, 0, nex.media.whiteShader);
	nexSyscall.R_DrawStretchPic(x, y + height - borderSize, width, borderSize, 0, 0, 0, 0, nex.media.whiteShader);

	nexSyscall.R_SetColor(NULL);
}

void CNexDraw::drawFillRect(float x, float y, float width, float height, const float *color) {
	nexSyscall.R_SetColor(color);

    adjustFrom640(&x, &y, &width, &height);
    nexSyscall.R_DrawStretchPic(x, y, width, height, 0, 0, 0, 1, nex.media.whiteShader);

	nexSyscall.R_SetColor(NULL);
}

void CNexDraw::drawPic(float x, float y, float width, float height, qhandle_t hShader) {
	adjustFrom640(&x, &y, &width, &height);
	nexSyscall.R_DrawStretchPic(x, y, width, height, 0, 0, 1, 1, hShader);
}

/**********************************
Credits to OGC for world2screen
**********************************/

/* finds angle between two vectors */
inline float VectorAngle(const vec3_t a, const vec3_t b)
{
    float length_a = VectorLength(a);
	float length_b = VectorLength(b);
	float length_ab = length_a*length_b;
	if( length_ab==0.0 ){ return 0.0; }
	else  { return (float) (acos(DotProduct(a,b)/length_ab) * (180.f/M_PI)); }
}

/* takes pitch/yaw and makes a vector */
void MakeVector(const vec3_t ain, vec3_t vout)
{
	float pitch;
	float yaw;
	float tmp;		
	
	pitch = (float) (ain[0] * M_PI/180);
	yaw = (float) (ain[1] * M_PI/180);
	tmp = (float) cos(pitch);
	
	vout[0] = (float) (-tmp * -cos(yaw));
	vout[1] = (float) (sin(yaw)*tmp);
	vout[2] = (float) -sin(pitch);
}

void VectorRotateX(const vec3_t in, float angle, vec3_t out)
{
	float a,c,s;

	a = (float) (angle * M_PI/180);
	c = (float) cos(a);
	s = (float) sin(a);
	out[0] = in[0];
	out[1] = c*in[1] - s*in[2];
	out[2] = s*in[1] + c*in[2];	
}

void VectorRotateY(const vec3_t in, float angle, vec3_t out)
{
	float a,c,s;

	a = (float) (angle * M_PI/180);
	c = (float) cos(a);
	s = (float) sin(a);
	out[0] = c*in[0] + s*in[2];
	out[1] = in[1];
	out[2] = -s*in[0] + c*in[2];
}

void VectorRotateZ(const vec3_t in, float angle, vec3_t out)
{
	float a,c,s;

	a = (float) (angle * M_PI/180);
	c = (float) cos(a);
	s = (float) sin(a);
	out[0] = c*in[0] - s*in[1];
	out[1] = s*in[0] + c*in[1];
	out[2] = in[2];
}


bool CNexDraw::worldToScreen(vec3_t worldCoord, int *x, int *y) {
	vec3_t aim;
	vec3_t newaim;
	vec3_t view;
	vec3_t tmp;
	float num;

	VectorCopy(nex.muzzle, tmp);
	VectorSubtract(worldCoord, tmp, aim);	
	MakeVector(nex.cg_refdefViewAngles, view);	

	if (VectorAngle(view, aim) > (nex.cg_refdef.fov_x / 1.8))
		return -1;
		
	VectorRotateZ(aim, -nex.cg_refdefViewAngles[1], newaim);
	VectorRotateY(newaim, -nex.cg_refdefViewAngles[0], tmp);
	VectorRotateX(tmp, -nex.cg_refdefViewAngles[2], newaim);
	
	if (newaim[0] <= 0)
		return false;
	
	num = (float) ((320.0f / newaim[0]) * (120.0 / nex.cg_refdef.fov_x - 1.0 / 3.0));

	*x = (int) (320 - num * newaim[1]);
	*y = (int) (240 - num * newaim[2]);


	BOUND_VALUE(*x, 0, 640);
	BOUND_VALUE(*y, 0, 480);

	return true;
}

void CNexDraw::drawText(float x, float y, float scale, char *text, vec4_t color, qboolean forceColor, qboolean shadow, fontInfo_t *font) {
	char *finalText = text;

	if (forceColor) {
		finalText = _strdup(text);
		Q_CleanStr(finalText);
	}

	CG_Text_Paint_Ext(x, y, scale, scale, color, finalText, 0.0f, 0, shadow ? ITEM_TEXTSTYLE_SHADOWED : 0, font);

	if (forceColor)
		free(finalText);
}

int CNexDraw::sizeTextHeight(float scale, fontInfo_t *font) {
	return (font->glyphs['['].height + 6) * font->glyphScale * scale;
}

int CNexDraw::sizeTextWidth(float scale, char *text, fontInfo_t *font) {
	return CG_Text_Width_Ext(text, scale, 0, font) + 1;
}

void CNexDraw::drawTextRect(float x, float y, float scale, char *text, vec4_t textColor, qboolean forceColor, qboolean textShadow, qboolean background, qboolean border, vec4_t backgroundColor, vec4_t borderColor, fontInfo_t *font) {
	int width = sizeTextWidth(scale, text, font);
	int height = sizeTextHeight(scale, font);
	float borderSize = font->glyphs['['].pitch * scale * 3;
	if (background)
		drawFillRect(x - borderSize, y - borderSize, width + (2 * borderSize), height + (2 * borderSize), backgroundColor);
	if (border)
		drawRect(x - borderSize, y - borderSize, width + (2 * borderSize), height + (2 * borderSize), borderColor, 1);

	drawText(x, y, scale, text, textColor, forceColor, textShadow, font);
}
