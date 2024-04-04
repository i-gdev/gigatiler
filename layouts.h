#ifndef LAYOUTS_H
#define LAYOUTS_H

/* Key binding functions */
static void defaultgaps(const Arg *arg);
static void incrgaps(const Arg *arg);
static void incrigaps(const Arg *arg);
static void incrogaps(const Arg *arg);
static void incrohgaps(const Arg *arg);
static void incrovgaps(const Arg *arg);
static void incrihgaps(const Arg *arg);
static void incrivgaps(const Arg *arg);
static void togglegaps(const Arg *arg);
/* Layouts */
static void bstack(Monitor *m);
static void bstackhoriz(Monitor *m);
static void centeredmaster(Monitor *m);
static void centeredfloatingmaster(Monitor *m);
static void deck(Monitor *m);
static void dwindle(Monitor *m);
static void fibonacci(Monitor *m, int s);
void flextile(Monitor *m);
static void gaplessgrid(Monitor *m);
static void grid(Monitor *m);
static void horizgrid(Monitor *m);
static void nrowgrid(Monitor *m);
static void spiral(Monitor *m);
void tile(Monitor *m);
/* Internals */
static void getgaps(Monitor *m, int *oh, int *ov, int *ih, int *iv, unsigned int *nc, float *mf, float *sf);
static void setgaps(int oh, int ov, int ih, int iv);
static void setflexsymbols(Monitor *m, unsigned int n);

/* Settings */
static int enablegaps = 1;

/* Named flextile constants */
#define LAYOUT 0
#define MASTER 1
#define STACK 2
#define SPLIT_VERTICAL 1   // master stack vertical split
#define SPLIT_HORIZONTAL 2 // master stack horizontal split
#define SPLIT_CENTERED_V 3 // centered master vertical split
#define SPLIT_CENTERED_H 4 // centered master horizontal split
#define LEFT_TO_RIGHT 1    // clients are stacked horizontally
#define TOP_TO_BOTTOM 2    // clients are stacked vertically
#define MONOCLE 3          // clients are stacked in deck / monocle mode
#define GRID 4             // clients are stacked in grid mode

#endif 