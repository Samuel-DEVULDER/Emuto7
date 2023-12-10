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

  video.h : header gestion video

*/

extern char	modif[1000];
extern int	conv[8000];
extern int      pa1,pb1;

void iniscreen(char *name);

void refreshv(void);

void refreshsc(void);

int  bouclex(void);

void effacewin(int i,long xm, long xx);

void scrollhaut(int cl,long xm, long xx, int i, int fl);

void scrollbas(int cl,long xm, long xx, int i, int fl);

/*
long loads(long k);

long loadl(long k);

int stocs(long k,long val);

int stocl(long k,long val);
*/
