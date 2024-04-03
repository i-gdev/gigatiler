typedef struct {
	Cursor cursor;
} Cur;

typedef struct Fnt {
	Display *dpy;
	unsigned int h;
	XftFont *xfont;
	FcPattern *pattern;
	struct Fnt *next;
} Fnt;

enum { ColFg, ColBg, ColBorder, ColFloat, ColCount }; /* Clr scheme index */
typedef XftColor Clr;

typedef struct {
	unsigned int w, h;
	Display *dpy;
	int screen;
	Window root;
	Drawable drawable;
	GC gc;
	Clr *scheme;
	Fnt *fonts;
} Ui;

/* Drawable abstraction */
Ui *ui_create(Display *dpy, int screen, Window win, unsigned int w, unsigned int h);
void ui_resize(Ui *ui, unsigned int w, unsigned int h);
void ui_free(Ui *ui);

/* Fnt abstraction */
Fnt *ui_fontset_create(Ui* ui, const char *fonts[], size_t fontcount);
void ui_font_getexts(Fnt *font, const char *text, unsigned int len, unsigned int *w, unsigned int *h);
void ui_fontset_free(Fnt* set);
unsigned int ui_fontset_getwidth(Ui *ui, const char *text, Bool markup);

/* Colorscheme abstraction */
void ui_clr_create(
	Ui *ui,
	Clr *dest,
	const char *clrname
);
Clr *ui_scm_create(
	Ui *ui,
	char *clrnames[],
	size_t clrcount
);

/* Cursor abstraction */
Cur *ui_cur_create(Ui *ui, int shape);
void ui_cur_free(Ui *ui, Cur *cursor);

/* Drawing context manipulation */
void ui_setfontset(Ui *ui, Fnt *set);
void ui_setscheme(Ui *ui, Clr *scm);

/* Drawing functions */
void ui_rect(Ui *ui, int x, int y, unsigned int w, unsigned int h, int filled, int invert);
int ui_text(Ui *ui, int x, int y, unsigned int w, unsigned int h, unsigned int lpad, const char *text, int invert, Bool markup);

/* Map functions */
void ui_map(Ui *ui, Window win, int x, int y, unsigned int w, unsigned int h);

