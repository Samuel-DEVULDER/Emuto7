/*
                    TTTTTTT    OOOOO    7777777
                       T      O     O        7
                       T      O     O       7
                       T      O     O      7
                       T      O     O     7
                       T       OOOOO     7

                             EMULATEUR

                          Par Sylvain HUET
                    (huet@poly.polytechnique.fr)
                               1994

  enterkey.c : definition des touches (compiler avec makekey)

  Adapation pour l'amiga par Samuel Devulder, 07/97.
*/

#include <stdio.h>

#include <proto/intuition.h>
#include <proto/exec.h>

#define nbkey 68

struct IntuitionBase    *IntuitionBase;
static struct Window    *W;

char		*nomstat=NULL;

void iniscreen()
{
  int	i,j,n;
  struct Screen *S;

  IntuitionBase = (void*)OpenLibrary("intuition.library",0L);
  if(!IntuitionBase) {
      fprintf(stderr,"No intuition.library ?\n");
      abort();
  }

  S = LockPubScreen(NULL);
  if(!S) {
      fprintf(stderr,"No public screen !\n");
      abort();
  }

  W = OpenWindowTags(NULL,
                       WA_Title,        (ULONG)"TO7 - Keep that window active",
                       WA_InnerWidth,   200,
                       WA_InnerHeight,  100,
                       WA_PubScreen,    (ULONG)S,
                       WA_IDCMP,        IDCMP_RAWKEY|
                                        0,
                       WA_Flags,        WFLG_ACTIVATE|
                                        0,
                       TAG_DONE);
      
  UnlockPubScreen(NULL, S);

  if(!W) {
      fprintf(stderr,"Can open window!\n");
      abort();
  }  
}

void closcreen()
{
    if(W) {CloseWindow(W);W=NULL;}
    if(IntuitionBase) {CloseLibrary((void*)IntuitionBase);IntuitionBase=NULL;}
}

char	cods[]=
"shift1 shift2 stop acc cnt ent raz haut gauche bas droite retour_coin ins eff \
1 2 3 4 5 6 7 8 9 0 \
A Z E R T Y U I O P \
Q S D F G H J K L M \
W X C V B N \
+ - * / @ . , space \
joystick0_haut gauche bas droite bouton \
joystick1_haut gauche bas droite bouton ";

char	*pnt;

void printkey()
{
  char c;
  
  do	{
    c=*(pnt++);
    putchar(c);
  }
  while(c!=32);
  fflush(stdout);
}

int WaitKey(void)
{
    struct IntuiMessage *msg;
    int k, f=0;
    do {
//	WaitIO((void*)W->UserPort);
	while(msg = (void*)GetMsg(W->UserPort)) {
	    k = msg->Code;
	    if(!f && msg->Class == IDCMP_RAWKEY &&
	       !(k&128)) {
		k = k&127;
		f = 1;
	    }
	    fflush(stdout);
	    ReplyMsg((void*)msg);
	}
    } while(!f);
    return k;
}

int main()
{
  int	i,j;
  int	key[256];
  FILE	*fl;
  atexit(closcreen);
  iniscreen();
  pnt=cods;
  for(j=0;j<nbkey;j++)
    {
      printf("\ntouche ");
      printkey();
      key[j]=WaitKey();
    }
  printf("\n");
  
  if (fl=fopen("keyto7","w"))
    {
      for(j=0;j<nbkey;j++)	fprintf(fl,"%d\n",key[j]);
      fclose(fl);
    }
  return 0;
}

