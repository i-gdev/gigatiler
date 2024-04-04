#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>

#include "ui.h"
#include "util.h"


#define UTF_INVALID 0xFFFD
#define UTF_SIZ     4

static const unsigned char utfbyte[UTF_SIZ + 1] = {0x80,    0, 0xC0, 0xE0, 0xF0};
static const unsigned char utfmask[UTF_SIZ + 1] = {0xC0, 0x80, 0xE0, 0xF0, 0xF8};
static const long utfmin[UTF_SIZ + 1] = {       0,    0,  0x80,  0x800,  0x10000};
static const long utfmax[UTF_SIZ + 1] = {0x10FFFF, 0x7F, 0x7FF, 0xFFFF, 0x10FFFF};


static long
utf8decodebyte(const char c, size_t *i)
{
	for (*i = 0; *i < (UTF_SIZ + 1); ++(*i))
		if (((unsigned char)c & utfmask[*i]) == utfbyte[*i])
			return (unsigned char)c & ~utfmask[*i];
	return 0;
}

static size_t
utf8validate(long *u, size_t i)
{
	if (!BETWEEN(*u, utfmin[i], utfmax[i]) || BETWEEN(*u, 0xD800, 0xDFFF))
		*u = UTF_INVALID;
	for (i = 1; *u > utfmax[i]; ++i)
		;
	return i;
}

static size_t
utf8decode(const char *c, long *u, size_t clen)
{
	size_t i, j, len, type;
	long udecoded;

	*u = UTF_INVALID;
	if (!clen)
		return 0;
	udecoded = utf8decodebyte(c[0], &len);
	if (!BETWEEN(len, 1, UTF_SIZ))
		return 1;
	for (i = 1, j = 1; i < clen && j < len; ++i, ++j) {
		udecoded = (udecoded << 6) | utf8decodebyte(c[i], &type);
		if (type)
			return j;
	}
	if (j < len)
		return 0;
	*u = udecoded;
	utf8validate(u, len);

	return len;
}

Ui *
ui_create(Display *dpy, int screen, Window root, unsigned int w, unsigned int h)
{
	Ui *ui = ecalloc(1, sizeof(Ui));

	ui->dpy = dpy;
	ui->screen = screen;
	ui->root = root;
	ui->w = w;
	ui->h = h;

	ui->drawable = XCreatePixmap(dpy, root, w, h, DefaultDepth(dpy, screen));
	ui->gc = XCreateGC(dpy, root, 0, NULL);
	XSetLineAttributes(dpy, ui->gc, 1, LineSolid, CapButt, JoinMiter);

	return ui;
}

void
ui_resize(Ui *ui, unsigned int w, unsigned int h)
{
	if (!ui)
		return;

	ui->w = w;
	ui->h = h;
	if (ui->drawable)
		XFreePixmap(ui->dpy, ui->drawable);
	ui->drawable = XCreatePixmap(ui->dpy, ui->root, w, h, DefaultDepth(ui->dpy, ui->screen));
}

void
ui_free(Ui *ui)
{
	XFreePixmap(ui->dpy, ui->drawable);
	XFreeGC(ui->dpy, ui->gc);
	ui_fontset_free(ui->fonts);
	free(ui);
}

/* This function is an implementation detail. Library users should use
 * ui_fontset_create instead.
 */
static Fnt *
xfont_create(Ui *ui, const char *fontname, FcPattern *fontpattern)
{
	Fnt *font;
	XftFont *xfont = NULL;
	FcPattern *pattern = NULL;

	if (fontname) {
		/* Using the pattern found at font->xfont->pattern does not yield the
		 * same substitution results as using the pattern returned by
		 * FcNameParse; using the latter results in the desired fallback
		 * behaviour whereas the former just results in missing-character
		 * rectangles being drawn, at least with some fonts. */
		if (!(xfont = XftFontOpenName(ui->dpy, ui->screen, fontname))) {
			fprintf(stderr, "error, cannot load font from name: '%s'\n", fontname);
			return NULL;
		}
		if (!(pattern = FcNameParse((FcChar8 *) fontname))) {
			fprintf(stderr, "error, cannot parse font name to pattern: '%s'\n", fontname);
			XftFontClose(ui->dpy, xfont);
			return NULL;
		}
	} else if (fontpattern) {
		if (!(xfont = XftFontOpenPattern(ui->dpy, fontpattern))) {
			fprintf(stderr, "error, cannot load font from pattern.\n");
			return NULL;
		}
	} else {
		die("no font specified.");
	}


	font = ecalloc(1, sizeof(Fnt));
	font->xfont = xfont;
	font->pattern = pattern;
	font->h = xfont->ascent + xfont->descent;
	font->dpy = ui->dpy;

	return font;
}

static void
xfont_free(Fnt *font)
{
	if (!font)
		return;
	if (font->pattern)
		FcPatternDestroy(font->pattern);
	XftFontClose(font->dpy, font->xfont);
	free(font);
}

Fnt*
ui_fontset_create(Ui* ui, const char *fonts[], size_t fontcount)
{
	Fnt *cur, *ret = NULL;
	size_t i;

	if (!ui || !fonts)
		return NULL;

	for (i = 1; i <= fontcount; i++) {
		if ((cur = xfont_create(ui, fonts[fontcount - i], NULL))) {
			cur->next = ret;
			ret = cur;
		}
	}
	return (ui->fonts = ret);
}

void
ui_fontset_free(Fnt *font)
{
	if (font) {
		ui_fontset_free(font->next);
		xfont_free(font);
	}
}

void
ui_clr_create(
	Ui *ui,
	Clr *dest,
	const char *clrname
) {
	if (!ui || !dest || !clrname)
		return;

	if (!XftColorAllocName(ui->dpy, DefaultVisual(ui->dpy, ui->screen),
	                       DefaultColormap(ui->dpy, ui->screen),
	                       clrname, dest))
		die("error, cannot allocate color '%s'", clrname);

}

/* Wrapper to create color schemes. The caller has to call free(3) on the
 * returned color scheme when done using it. */
Clr *
ui_scm_create(
	Ui *ui,
	char *clrnames[],
	size_t clrcount
) {
	size_t i;
	Clr *ret;

	/* need at least two colors for a scheme */
	if (!ui || !clrnames || clrcount < 2 || !(ret = ecalloc(clrcount, sizeof(XftColor))))
		return NULL;

	for (i = 0; i < clrcount; i++)
		ui_clr_create(ui, &ret[i], clrnames[i]);
	return ret;
}

void
ui_setfontset(Ui *ui, Fnt *set)
{
	if (ui)
		ui->fonts = set;
}

void
ui_setscheme(Ui *ui, Clr *scm)
{
	if (ui)
		ui->scheme = scm;
}


void
ui_rect(Ui *ui, int x, int y, unsigned int w, unsigned int h, int filled, int invert)
{
	if (!ui || !ui->scheme)
		return;
	XSetForeground(ui->dpy, ui->gc, invert ? ui->scheme[ColBg].pixel : ui->scheme[ColFg].pixel);
	if (filled)
		XFillRectangle(ui->dpy, ui->drawable, ui->gc, x, y, w, h);
	else
		XDrawRectangle(ui->dpy, ui->drawable, ui->gc, x, y, w - 1, h - 1);
}

int
ui_text(Ui *ui, int x, int y, unsigned int w, unsigned int h, unsigned int lpad, const char *text, int invert)
{
	int i, ty, ellipsis_x = 0;
	unsigned int tmpw, ew, ellipsis_w = 0, ellipsis_len;
	XftDraw *d = NULL;
	Fnt *usedfont, *curfont, *nextfont;
	int utf8strlen, utf8charlen, render = x || y || w || h;
	long utf8codepoint = 0;
	const char *utf8str;
	FcCharSet *fccharset;
	FcPattern *fcpattern;
	FcPattern *match;
	XftResult result;
	int charexists = 0, overflow = 0;
	/* keep track of a couple codepoints for which we have no match. */
	enum { nomatches_len = 64 };
	static struct { long codepoint[nomatches_len]; unsigned int idx; } nomatches;
	static unsigned int ellipsis_width = 0;

	if (!ui || (render && (!ui->scheme || !w)) || !text || !ui->fonts)
		return 0;

	if (!render) {
		w = invert ? invert : ~invert;
	} else {
		XSetForeground(ui->dpy, ui->gc, ui->scheme[invert ? ColFg : ColBg].pixel);
		XFillRectangle(ui->dpy, ui->drawable, ui->gc, x, y, w, h);
		d = XftDrawCreate(ui->dpy, ui->drawable,
		                  DefaultVisual(ui->dpy, ui->screen),
		                  DefaultColormap(ui->dpy, ui->screen));
		x += lpad;
		w -= lpad;
	}

	usedfont = ui->fonts;
	if (!ellipsis_width && render)
		ellipsis_width = ui_fontset_getwidth(ui, "...");
	while (1) {
		ew = ellipsis_len = utf8strlen = 0;
		utf8str = text;
		nextfont = NULL;
		while (*text) {
			utf8charlen = utf8decode(text, &utf8codepoint, UTF_SIZ);
			for (curfont = ui->fonts; curfont; curfont = curfont->next) {
				charexists = charexists || XftCharExists(ui->dpy, curfont->xfont, utf8codepoint);
				if (charexists) {
					ui_font_getexts(curfont, text, utf8charlen, &tmpw, NULL);
					if (ew + ellipsis_width <= w) {
						/* keep track where the ellipsis still fits */
						ellipsis_x = x + ew;
						ellipsis_w = w - ew;
						ellipsis_len = utf8strlen;
					}

					if (ew + tmpw > w) {
						overflow = 1;
						/* called from ui_fontset_getwidth_clamp():
						 * it wants the width AFTER the overflow
						 */
						if (!render)
							x += tmpw;
						else
							utf8strlen = ellipsis_len;
					} else if (curfont == usedfont) {
						utf8strlen += utf8charlen;
						text += utf8charlen;
						ew += tmpw;
					} else {
						nextfont = curfont;
					}
					break;
				}
			}

			if (overflow || !charexists || nextfont)
				break;
			else
				charexists = 0;
		}

		if (utf8strlen) {
			if (render) {
				ty = y + (h - usedfont->h) / 2 + usedfont->xfont->ascent;
				XftDrawStringUtf8(d, &ui->scheme[invert ? ColBg : ColFg],
				                  usedfont->xfont, x, ty, (XftChar8 *)utf8str, utf8strlen);
			}
			x += ew;
			w -= ew;
		}
		if (render && overflow)
			ui_text(ui, ellipsis_x, y, ellipsis_w, h, 0, "...", invert);

		if (!*text || overflow) {
			break;
		} else if (nextfont) {
			charexists = 0;
			usedfont = nextfont;
		} else {
			/* Regardless of whether or not a fallback font is found, the
			 * character must be drawn. */
			charexists = 1;

			for (i = 0; i < nomatches_len; ++i) {
				/* avoid calling XftFontMatch if we know we won't find a match */
				if (utf8codepoint == nomatches.codepoint[i])
					goto no_match;
			}

			fccharset = FcCharSetCreate();
			FcCharSetAddChar(fccharset, utf8codepoint);

			if (!ui->fonts->pattern) {
				/* Refer to the comment in xfont_create for more information. */
				die("the first font in the cache must be loaded from a font string.");
			}

			fcpattern = FcPatternDuplicate(ui->fonts->pattern);
			FcPatternAddCharSet(fcpattern, FC_CHARSET, fccharset);
			FcPatternAddBool(fcpattern, FC_SCALABLE, FcTrue);

			FcConfigSubstitute(NULL, fcpattern, FcMatchPattern);
			FcDefaultSubstitute(fcpattern);
			match = XftFontMatch(ui->dpy, ui->screen, fcpattern, &result);

			FcCharSetDestroy(fccharset);
			FcPatternDestroy(fcpattern);

			if (match) {
				usedfont = xfont_create(ui, NULL, match);
				if (usedfont && XftCharExists(ui->dpy, usedfont->xfont, utf8codepoint)) {
					for (curfont = ui->fonts; curfont->next; curfont = curfont->next)
						; /* NOP */
					curfont->next = usedfont;
				} else {
					xfont_free(usedfont);
					nomatches.codepoint[++nomatches.idx % nomatches_len] = utf8codepoint;
no_match:
					usedfont = ui->fonts;
				}
			}
		}
	}
	if (d)
		XftDrawDestroy(d);

	return x + (render ? w : 0);
}

void
ui_map(Ui *ui, Window win, int x, int y, unsigned int w, unsigned int h)
{
	if (!ui)
		return;

	XCopyArea(ui->dpy, ui->drawable, win, ui->gc, x, y, w, h, x, y);
	XSync(ui->dpy, False);
}

unsigned int
ui_fontset_getwidth(Ui *ui, const char *text)
{
	if (!ui || !ui->fonts || !text)
		return 0;
	return ui_text(ui, 0, 0, 0, 0, 0, text, 0);
}

unsigned int
ui_fontset_getwidth_clamp(Ui *ui, const char *text, unsigned int n)
{
	unsigned int tmp = 0;
	if (ui && ui->fonts && text && n)
		tmp = ui_text(ui, 0, 0, 0, 0, 0, text, n);
	return MIN(n, tmp);
}

void
ui_font_getexts(Fnt *font, const char *text, unsigned int len, unsigned int *w, unsigned int *h)
{
	XGlyphInfo ext;

	if (!font || !text)
		return;

	XftTextExtentsUtf8(font->dpy, font->xfont, (XftChar8 *)text, len, &ext);
	if (w)
		*w = ext.xOff;
	if (h)
		*h = font->h;
}

Cur *
ui_cur_create(Ui *ui, int shape)
{
	Cur *cur;

	if (!ui || !(cur = ecalloc(1, sizeof(Cur))))
		return NULL;

	cur->cursor = XCreateFontCursor(ui->dpy, shape);

	return cur;
}

void
ui_cur_free(Ui *ui, Cur *cursor)
{
	if (!cursor)
		return;

	XFreeCursor(ui->dpy, cursor->cursor);
	free(cursor);
}

