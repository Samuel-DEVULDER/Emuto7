/*
                    TTTTTTT    OOOOO    7777777
                       T      O     O        7
                       T      O     O       7     77777  000
                       T      O     O      7         7  0   0
                       T      O     O     7         7   0   0
                       T       OOOOO     7         7     000

                             EMULATEUR

                          Par Sylvain HUET
                    (Sylvain.Huet@inria.fr)
                               1996

  monitor.h : header du moniteur

*/

#include <stdio.h>
#include <stdlib.h>

#define sizebuf 1024*1024

extern  char    buf[sizebuf];
extern  long    leng;
extern  long    ind;
extern  long    offs;
extern  int     nbo;
extern  int     mode;
extern  long    follow;
extern  int     taille[];
extern  int     adr[];
extern  long    compt;
extern  int     quitf;
extern  FILE    *fk7;
extern  int     modegraph;
extern  long    mask;
extern  int     flto7;
extern  long    endmem;

void ini6809(void);

int des6809(long k, char *c);

void inito7(char *name);

void newcass(char *c);

void avanck7(long k);

void refreshsc(void);

void refreshv(void);

int bouclex(void);

void exe6809(void);

void printreg(void);

void printind(void);

void setreg(char *c,long k);

void printhx(long k,int n);

void printad(long k);

void copyto7(void);

void convbasic(void);

void catdsk(void);

/*
void displaypath(void);
void displaydir(void);
*/
