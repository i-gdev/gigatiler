#ifndef LAYOUTS_H
#define LAYOUTS_H

#include "gigatiler.h"

/* Key binding functions */

/* Layouts */
void centeredmaster(Monitor *m);
void grid(Monitor *m);
void tile(Monitor *m);
/* Internals */
void getgaps(Monitor *m, int *oh, int *ov, int *ih, int *iv, unsigned int *nc, float *mf, float *sf);
void setgaps(int oh, int ov, int ih, int iv);
void setflexsymbols(Monitor *m, unsigned int n);
void defaultgaps(const Arg *arg);
void incrgaps(const Arg *arg);
void incrigaps(const Arg *arg);
void incrogaps(const Arg *arg);
void incrohgaps(const Arg *arg);
void incrovgaps(const Arg *arg);
void incrihgaps(const Arg *arg);
void incrivgaps(const Arg *arg);
void togglegaps(const Arg *arg);


/* Settings */
static int enablegaps = 1;

#endif 