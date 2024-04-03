#ifndef GIGATILER_H
#define GIGATILER_H

#define Button6                 6
#define Button7                 7
#define Button8                 8
#define Button9                 9
#define NUMTAGS                 9
#define NUMVIEWHIST             NUMTAGS
#define BARRULES                20
#define BUTTONMASK              (ButtonPressMask|ButtonReleaseMask)
#define CLEANMASK(mask)         (mask & ~(numlockmask|LockMask) & (ShiftMask|ControlMask|Mod1Mask|Mod2Mask|Mod3Mask|Mod4Mask|Mod5Mask))
#define INTERSECT(x,y,w,h,m)    (MAX(0, MIN((x)+(w),(m)->mx+(m)->mw) - MAX((x),(m)->mx)) \
                               * MAX(0, MIN((y)+(h),(m)->my+(m)->mh) - MAX((y),(m)->my)))
#define ISVISIBLEONTAG(C, T)    ((C->tags & T))
#define ISVISIBLE(C)            ISVISIBLEONTAG(C, C->mon->tagset[C->mon->seltags])
#define LENGTH(X)               (sizeof X / sizeof X[0])
#define MOUSEMASK               (BUTTONMASK|PointerMotionMask)
#define WIDTH(X)                ((X)->w + 2 * (X)->bw)
#define HEIGHT(X)               ((X)->h + 2 * (X)->bw)
#define WTYPE                   "_NET_WM_WINDOW_TYPE_"
#define TOTALTAGS               (NUMTAGS + LENGTH(scratchpads))
#define TAGMASK                 ((1 << TOTALTAGS) - 1)
#define SPTAG(i)                ((1 << NUMTAGS) << (i))
#define SPTAGMASK               (((1 << LENGTH(scratchpads))-1) << NUMTAGS)
#define TEXTWM(X)               (ui_fontset_getwidth(ui, (X), True) + lrpad)
#define TEXTW(X)                (ui_fontset_getwidth(ui, (X), False) + lrpad)
#define HIDDEN(C)               ((getstate(C->win) == IconicState))


/* enums */
enum {
	CurNormal,
	CurResize,
	CurMove,
	CurLast
}; /* cursor */

enum {
	SchemeNorm,
	SchemeSel,
	SchemeTitleNorm,
	SchemeTitleSel,
	SchemeTagsNorm,
	SchemeTagsSel,
	SchemeHidNorm,
	SchemeHidSel,
	SchemeUrg,
}; /* color schemes */

enum {
	NetSupported, NetWMName, NetWMState, NetWMCheck,
	NetWMFullscreen, NetActiveWindow, NetWMWindowType,
	NetDesktopNames, NetDesktopViewport, NetNumberOfDesktops, NetCurrentDesktop,
	NetClientList,
	NetClientListStacking,
	NetLast
}; /* EWMH atoms */

enum {
	WMProtocols,
	WMDelete,
	WMState,
	WMTakeFocus,
	WMWindowRole,
	WMLast
}; /* default atoms */

enum {
	ClientFields,
	ClientTags,
	ClientLast
}; /* gigatiler client atoms */

enum {
	ClkTagBar,
	ClkLtSymbol,
	ClkStatusText,
	ClkWinTitle,
	ClkClientWin,
	ClkRootWin,
	ClkLast
}; /* clicks */

enum {
	BAR_ALIGN_LEFT,
	BAR_ALIGN_CENTER,
	BAR_ALIGN_RIGHT,
	BAR_ALIGN_LEFT_LEFT,
	BAR_ALIGN_LEFT_RIGHT,
	BAR_ALIGN_LEFT_CENTER,
	BAR_ALIGN_NONE,
	BAR_ALIGN_RIGHT_LEFT,
	BAR_ALIGN_RIGHT_RIGHT,
	BAR_ALIGN_RIGHT_CENTER,
	BAR_ALIGN_LAST
}; /* bar alignment */

typedef struct TagState TagState;
struct TagState {
       int selected;
       int occupied;
       int urgent;
};

typedef struct ClientState ClientState;
struct ClientState {
       int isfixed, isfloating, isurgent, neverfocus, oldstate, isfullscreen;
};

typedef union {
	long i;
	unsigned long ui;
	float f;
	const void *v;
} Arg;

typedef struct Monitor Monitor;
typedef struct Bar Bar;
struct Bar {
	Window win;
	Monitor *mon;
	Bar *next;
	int idx;
	int showbar;
	int topbar;
	int external;
	int borderpx;
	int borderscheme;
	int bx, by, bw, bh; /* bar geometry */
	int w[BARRULES]; // width, array length == barrules, then use r index for lookup purposes
	int x[BARRULES]; // x position, array length == ^
};

typedef struct {
	int x;
	int y;
	int h;
	int w;
} BarArg;

typedef struct {
	int monitor;
	int bar;
	int alignment; // see bar alignment enum
	int (*widthfunc)(Bar *bar, BarArg *a);
	int (*drawfunc)(Bar *bar, BarArg *a);
	int (*clickfunc)(Bar *bar, Arg *arg, BarArg *a);
	int (*hoverfunc)(Bar *bar, BarArg *a, XMotionEvent *ev);
	char *name; // for debugging
	int x, w; // position, width for internal use
} BarRule;

typedef struct {
	unsigned int click;
	unsigned int mask;
	unsigned int button;
	void (*func)(const Arg *arg);
	const Arg arg;
} Button;


typedef struct Client Client;
struct Client {
	char name[256];
	float mina, maxa;
	float cfact;
	int x, y, w, h;
	unsigned int idx;
	int oldx, oldy, oldw, oldh;
	int basew, baseh, incw, inch, maxw, maxh, minw, minh, hintsvalid;
	int bw, oldbw;
	unsigned int tags;
	unsigned int switchtag;
	int isfixed, isfloating, isurgent, neverfocus, oldstate, isfullscreen;
	int iscentered;
	int beingmoved;
	int isterminal, noswallow;
	pid_t pid;
	Client *next;
	Client *snext;
	Client *swallowing;
	Monitor *mon;
	Window win;
	ClientState prevstate;
};

typedef struct {
	unsigned int mod;
	KeySym keysym;
	void (*func)(const Arg *);
	const Arg arg;
} Key;


typedef struct {
	const char *symbol;
	void (*arrange)(Monitor *);
} Layout;


typedef struct Pertag Pertag;
struct Monitor {
	char ltsymbol[16];
	float mfact;
	int nmaster;
	int num;
	int mx, my, mw, mh;   /* screen size */
	int wx, wy, ww, wh;   /* window area  */
	int gappih;           /* horizontal gap between windows */
	int gappiv;           /* vertical gap between windows */
	int gappoh;           /* horizontal outer gaps */
	int gappov;           /* vertical outer gaps */
	unsigned int seltags;
	unsigned int sellt;
	unsigned int tagset[2];
	int showbar;
	Client *clients;
	Client *sel;
	Client *stack;
	Monitor *next;
	Bar *bar;
	const Layout *lt[2];
	Pertag *pertag;
	char lastltsymbol[16];
	TagState tagstate;
	Client *lastsel;
	const Layout *lastlt;
};

typedef struct {
	const char *class;
	const char *role;
	const char *instance;
	const char *title;
	const char *wintype;
	unsigned int tags;
	int switchtag;
	int iscentered;
	int isfloating;
	int isterminal;
	int noswallow;
	int monitor;
} Rule;

#define RULE(...) { .monitor = -1, __VA_ARGS__ },

/* Cross patch compatibility rule macro helper macros */
#define FLOATING , .isfloating = 1
#define CENTERED , .iscentered = 1
#define PERMANENT
#define FAKEFULLSCREEN
#define NOSWALLOW , .noswallow = 1
#define TERMINAL , .isterminal = 1
#define SWITCHTAG , .switchtag = 1

typedef struct {
	int monitor;
	int tag;
	int layout;
	float mfact;
	int nmaster;
	int showbar;
	int topbar;
} MonitorRule;


#endif