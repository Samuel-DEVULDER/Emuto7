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

*/

#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/keysym.h>

#define nbkey 68

int		black,white;
char		*nomstat=NULL;
Display		*display;
int		screen;
Colormap	cmap;
Visual		*visual;
Window		root,win;
GC		gc;

void iniscreen()
{
  int	i,j,n;
  XGCValues xgc;
  display=XOpenDisplay(nomstat);
  screen=DefaultScreen(display);
  visual=DefaultVisual(display,screen);
  cmap=DefaultColormap(display,screen);
  root=RootWindow(display,screen);
  black=BlackPixel(display,screen);
  white=WhitePixel(display,screen);
  gc=DefaultGC(display,screen);
  
  win=XCreateSimpleWindow(display, root,0,0,640,400,1,white,black);
  XSetStandardProperties(display, win, "To7-clavier","To7-clavier",0,
                         NULL,0,NULL);
  XSetWindowColormap(display,win,cmap);
  XSelectInput(display,win,KeyPressMask);
  XMapWindow(display,win);
  
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

void main()
{
  XEvent	ev;
  int	i,j;
  int	key[256];
  FILE	*fl;
  iniscreen();
  pnt=cods;
  for(j=0;j<nbkey;j++)
    {
      printf("\ntouche ");
      printkey();
      XNextEvent(display,&ev);
      switch(ev.type)	{
      case KeyPress:
        key[j]=ev.xkey.keycode&255;
        break;
      default :
        break;	}
    }
  printf("\n");
  
  if (fl=fopen("keyto7","w"))
    {
      for(j=0;j<nbkey;j++)	fprintf(fl,"%d\n",key[j]);
      fclose(fl);
    }
}

