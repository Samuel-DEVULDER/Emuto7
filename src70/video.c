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

  video.c : routines video de l'emulateur (+gestion memoire)

*/

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/keysym.h>

#include "monitor.h"
#include "emuto7.h"

#define nbtouche 68
#define nkey 3

int		black,white;
char		*nomstat=NULL;
Display		*display;
int		screen;
Colormap	cmap;
Visual		*visual;
Window		root,win;
GC		gc,mygc[256];
short		imac[400],codedb[256];
XImage		*ima;
XColor		tCol,tCol_exact;
static char	*nom[16]=
{"gray","pink","pale green","light yellow","light slate blue","violet"
,"light cyan","orange"
,"black","red","green","yellow","blue","magenta","cyan","white"};
static char	*nommono[16]=
{ "black","gray15","gray30","gray45","gray60","gray75","gray90","white"
,"black","gray15","gray30","gray45","gray60","gray75","gray90","white"};
int		col[16];

char		modif[1040];
int		conv[8192];

int	tch[nkey];
int	lkey[256];
int	ckey[256];
FILE	*fl;
FILE	*fk7;
int	crayx,crayy;
int	lasttour;

int nbwait;

int gainit;
int pa1,pb1;

int	pain1,paout1,crain1;
int	pbin1,pbout1,crbin1;

int	pain2,paout2,crain2;
int	pbin2,pbout2,crbin2;

int	codel[]=
{0x01,0x01,0x02,0x02,0x02,0x02,0x02,0x02,0x04,0x08,0x10,0x20,0x40,0x80
   ,0x04,0x08,0x10,0x20,0x40,0x80,0x80,0x40,0x20,0x10
     ,0x04,0x08,0x10,0x20,0x40,0x80,0x80,0x40,0x20,0x10
       ,0x04,0x08,0x10,0x20,0x40,0x80,0x80,0x40,0x20,0x10
         ,0x02,0x04,0x02,0x04,0x08,0x80
           ,0x04,0x08,0x04,0x08,0x10,0x20,0x40,0x08
             ,0x00,0x00,0x00,0x00,0x00
               ,0x00,0x00,0x00,0x00,0x00
};
int	codec[]=
{0x01,0x01,0x80,0x40,0x20,0x10,0x08,0x02,0x02,0x02,0x02,0x02,0x02,0x02
   ,0x80,0x80,0x80,0x80,0x80,0x80,0x40,0x40,0x40,0x40
     ,0x20,0x20,0x20,0x20,0x20,0x20,0x10,0x10,0x10,0x10
       ,0x08,0x08,0x08,0x08,0x08,0x08,0x04,0x04,0x04,0x04
         ,0x01,0x01,0x04,0x04,0x04,0x01
           ,0x40,0x40,0x10,0x10,0x01,0x01,0x01,0x01
             ,0xfe,0xfb,0xfd,0xf7,0x1
               ,0xef,0xbf,0xdf,0x7f,0x2
};

void inicray()
{
  crayx=-1;
}

void inikey(char *name)
{
  int	i,key;
  
  for(i=0;i<256;i++)
    {
      lkey[i]=0;
      ckey[i]=255;
    }
  tch[0]=tch[1]=0;
  
  if (fl=fopen(name,"r"))
    {
      for(i=0;i<nbtouche;i++)
        {
          fscanf(fl,"%d",&key);
          lkey[key&255]=codel[i];
          ckey[key&255]=(~codec[i])&255;
        }
      fclose(fl);
    }
  else
    {
      printf("lancer d'abord enterkey\n");
      exit(0);
    }
  buf[0xe7cc]=0;
  buf[0xe7cd]=0;
  buf[0xe7ce]=0;
  buf[0xe7cf]=0;
}

void newcass(char *c)
{
  if (fk7) fclose(fk7);
  if ((fk7=fopen(c,"r+"))==NULL) fk7=fopen(c,"w+");
  if (fk7==NULL) if (fk7=fopen(c,"r")) printf("LECTURE SEULE\n");
  if (fk7==NULL) printf("impossible\n");
  else
    printf("cassette %s\n",c);
}

void avanck7(long k)
{
  if (fk7==NULL)
    printf("impossible\n");
  else
    {
      fseek(fk7,k,0);
      printhx(ftell(fk7),4);
      printf(" : nouvelle position\n");
    }
}

void inik7()
{
  fk7=NULL;
}

void iniscreen(char *name)
{
  int	i,j,k,l,n;
  XGCValues xgc;
  display=XOpenDisplay(nomstat);
  screen=DefaultScreen(display);
  visual=DefaultVisual(display,screen);
  cmap=DefaultColormap(display,screen);
  root=RootWindow(display,screen);
  black=BlackPixel(display,screen);
  white=WhitePixel(display,screen);
  gc=DefaultGC(display,screen);
  
  if(modegraph==0)
    {
      for(i=0;i<256;i++)
        {
          mygc[i]=XCreateGC(display,root,0,&xgc);
          XSetForeground(display,mygc[i],white);
          XSetBackground(display,mygc[i],black);
        }
    }
  if(modegraph==1)
    {
      for (i=0;i<16;i++)
        {
          XAllocNamedColor(display,cmap,nom[i],&tCol,&tCol_exact);
          col[i]=tCol.pixel;
        }
      
      for(k=0;k<2;k++)
	for(l=0;l<2;l++)
          for(i=0;i<8;i++)
            for(j=0;j<8;j++)
              {
		mygc[128*l+64*k+8*i+j]=XCreateGC(display,root,0,&xgc);
		XSetForeground(display,mygc[128*l+64*k+8*i+j],col[k*8+i]);
		XSetBackground(display,mygc[128*l+64*k+8*i+j],col[l*8+j]);
              }
    }
  if(modegraph==2)
    {
      printf("a\n");
      for (i=0;i<16;i++)
        {
          XAllocNamedColor(display,cmap,nommono[i],&tCol,&tCol_exact);
          col[i]=tCol.pixel;
        }
      
      printf("b\n");
      for(k=0;k<2;k++)
	for(l=0;l<2;l++)
          for(i=0;i<8;i++)
            for(j=0;j<8;j++)
              {
		mygc[128*l+64*k+8*i+j]=XCreateGC(display,root,0,&xgc);
		XSetForeground(display,mygc[128*l+64*k+8*i+j],col[k*8+i]);
		XSetBackground(display,mygc[128*l+64*k+8*i+j],col[l*8+j]);
              }
    }
  
  win=XCreateSimpleWindow(display, root,0,0,680,440,1,white,black);
  XSetStandardProperties(display, win, "To7-70","To7-70",0,
                         NULL,0,NULL);
  XSetWindowColormap(display,win,cmap);
  XSelectInput(display,win,ExposureMask|ButtonPressMask|ButtonReleaseMask
               |Button2MotionMask|KeyPressMask|KeyReleaseMask);
  XMapWindow(display,win);
  
  ima=XCreateImage(display,visual,
                   1,XYBitmap,0,(char *) imac,16,16*25,16,2);
  for(i=0;i<256;i++)
    {
      for (j=7,n=i;j>=0;j--,n>>=1)
        {
          XPutPixel(ima,j*2,0,n&1);
          XPutPixel(ima,j*2+1,0,n&1);
        }
      codedb[i]=imac[0];
    }
  XFlush(display);
  
  for(i=0;i<8192;i++) conv[i]=(i%40)*25+(i/320);
  inikey(name);
  inicray();
  inik7();
  lasttour=4;
}

void refreshv()
{
  char	*fr;
  char	*fd;
  char	*p;
  char	*q;
  int	x,y;
  int	j;
  int	cl;
  char	*tes;
  
  fr=&buf[0x4000];
  fd=&buf[0x10000];
  tes=modif;
  for(x=0;x<640;x+=16)
    {
      j=0;
      cl=*fd;
      p=fr;
      q=fd;
      for(y=0;y<400;y+=2)
        {
          if (!(y&15))
            {
              if (*tes)	*(tes++)=0;
              else	
                {
                if (y-j) XPutImage(display,win
                                   ,mygc[(cl&255)],ima,0,0,x+20,j+20,16,y-j);
                y+=14;
                p+=320;
                q+=320;
                cl=*q;
                j=y+2;
                tes++;
                continue;
              }	
            }
          if ((*q)!=cl)
            {
              XPutImage(display,win,mygc[(cl&255)]
                        ,ima,0,0,x+20,j+20,16,y-j);
              j=y;
              cl=*q;
            }
          imac[y-j]=imac[y+1-j]=codedb[((*p)&255)];
          p+=40;
          q+=40;
          
        }
      if(y-j) XPutImage(display,win
                        ,mygc[(cl&255)],ima,0,0,x+20,j+20,16,y-j);
      fr+=1;
      fd+=1;
    }
  if ((modegraph)&&(lasttour!=(buf[0xe7c3]&0x74)))
    {
      lasttour=(buf[0xe7c3]&0x74);
      cl=(lasttour&0x70)+((lasttour&4)<<5);
      cl=cl>>1;
      XFillRectangle(display,win,mygc[cl],0,0,680,20);
      XFillRectangle(display,win,mygc[cl],0,20,20,400);
      XFillRectangle(display,win,mygc[cl],660,20,20,400);
      XFillRectangle(display,win,mygc[cl],0,420,680,20);
    }
  XFlush(display);
}

void refreshsc()
{
  int	i;
  
  for(i=0;i<1000;i++) modif[i]=1;
  lasttour=4;
  refreshv();
}

long REGPARM loads(long k)
{
  int i,j,pb;
  
  if ((k&0xffc0)==0xe7C0)
    {               /* gestion des io       */
      if (k==0xe7c0) return 0x81;
      
      if ((k&0xfffc)==0xe7c8)
	{
	  if ((k==0xe7c8)&&(buf[0xe7ca]&4))
	    {
	      pb=(0x80>>(pbout1&7));
	      pain1=0xff;
	      for(i=0;i<nkey;i++)
		pain1&=((pb&lkey[tch[i]])?ckey[tch[i]]:0xff);
	      return ((paout1&buf[0xe7c8])+(pain1&(~buf[0xe7c8])))&255;
	    }
	  if ((k==0xe7c9)&&(buf[0xe7cb]&4))
	    return ((pbout1&buf[0xe7c9])+(pbin1&(~buf[0xe7c9])))&255;
	  if (k==0xe7ca) return (buf[0xe7ca]&0x7f)+(crain1&0x80);
	  if (k==0xe7cb) return (buf[0xe7cb]&0x7f)+(crbin1&0x80);
	}
      
      if ((k&0xfffc)==0xe7cc)
	{
	  if ((k==0xe7cc)&&(buf[0xe7ce]&4))
	    return ((paout2&buf[0xe7cc])+(pain2&(~buf[0xe7cc])))&255;
	  if ((k==0xe7cd)&&(buf[0xe7cf]&4))
	    return ((pbout2&buf[0xe7cd])+(pbin2&(~buf[0xe7cd])))&255;
	  if (k==0xe7ce) return (buf[0xe7ce]&0x7f)+(crain2&0x80);
	  if (k==0xe7cf) return (buf[0xe7cf]&0x7f)+(crbin2&0x80);
	  
	}
      if (k==0xe7e7)
	{
	  if (gainit>=6666) gainit-=6666;
	  if (gainit>=4266) return 0;
	  return 128;
	}	
      return buf[k]&255;
    }
  if ((k>=0xa000)&&(k<0xe000))
    {
      i=0;
      if (buf[0xe7c9]&8) i|=buf[k];
      if (buf[0xe7c9]&0x10) i|=buf[k+0x8000];
      if (buf[0xe7c9]&0x18) return i&255;
      return 0x5e;
    }
  if (((k&0xe000)==0x4000) && offset_fond) k+=0xc000;
    return buf[k]&255;
}

long REGPARM loadl(long k)
{
  int i;
  
  if ((k&0xffc0)==0xe7C0)
    {               /* gestion des io       */
      return ((loads(k)<<8)+(loads(k+1)&255))&0xffff;
    }
  if ((k>=0xa000)&&(k<0xe000))
    {
      i=0;
      if (buf[0xe7c9]&8) i|=((buf[k]&255)<<8)+(buf[k+1]&255);
      if (buf[0xe7c9]&0x10) i|=((buf[k+0x8000]&255)<<8)+(buf[k+0x8000+1]&255);
      if (buf[0xe7c9]&0x18) return i&0xffff;
      return 0x5e5e;
    }
  if (((k&0xe000)==0x4000)&&(!(buf[0xe7c3]&1))) k+=0xc000;
  return ((buf[k]&255)<<8)+(buf[k+1]&255);
}

int REGPARM stocs(long k,long val)
{
  if (k<0x6000)
    {
      if (k<0x4000) return 0;
      modif[conv[k-0x4000]]=1;
      if (!(buf[0xe7c3]&1)) k+=0xc000;
      buf[k]=val;
      return 0;
    }
  if (k>endmem)
    {
      if ((k&0xffc0)==0xe7C0)
	{
	  if ((k==0xe7c8)&&(buf[0xe7ca]&4))
	    {
	      paout1=val;
	      return 0;
	    }
	  if ((k==0xe7c9)&&(buf[0xe7cb]&4))
	    {
	      pbout1=val;
	      return 0;
	    }  
	  
	  if ((k==0xe7cc)&&(buf[0xe7ce]&4))
	    {
	      paout2=val;
	      return 0;
	    }	
	  if ((k==0xe7cd)&&(buf[0xe7cf]&4))
	    {
	      pbout2=val;
	      return 0;
	    }	
	  
	  if (k==0xe7c3) {val|=128;offset_fond=(val&1)?0:0xc000;}
	  buf[k]=val;
	} 
      return 0;
    }
  
  if (k>=0xa000)
    {
      if (buf[0xe7c9]&8) buf[k]=val;
      if (buf[0xe7c9]&0x10) buf[k+0x8000]=val;
      return 0;
    }
  buf[k]=val;
  return 0;
}

int REGPARM stocl(long k,long val)
{
  if (k<0x6000)
    {
      if (k<0x4000) return 0;
      modif[conv[k-0x4000]]=1;
      modif[conv[k+1-0x4000]]=1;
      if (shift_fond/*!(buf[0xe7c3]&1)*/) k+=0xc000;
      buf[k+1]=val;
      val>>=8;
      buf[k]=val;
      return 0;
    }
  if (k>endmem)
    {
      if ((k&0xffc0)==0xe7C0)
        {               /* gestion des io       */
          stocs(k,val>>8);
          stocs(k+1,val);
        }	
      return 0;
    }
  if (k>=0xa000)
    {
      if (buf[0xe7c9]&8) buf[k+1]=val;
      if (buf[0xe7c9]&0x10) buf[k+0x8000+1]=val;
      val>>=8;
      if (buf[0xe7c9]&8) buf[k]=val;
      if (buf[0xe7c9]&0x10) buf[k+0x8000]=val;
      return 0;
    }
  buf[k+1]=val;
  val>>=8;
  buf[k]=val;
  return 0;
}


long	compt;
int	quitf;

int bouclex()
{
  XEvent	ev;
  int	i,j,k;
  int	key;
  int   ninst;

  j=0;
  compt=0;
  for(i=0;i<nkey;i++) tch[i]=0;
  quitf=1;

  pain2=paout2=crain2=255;
  pbin2=pbout2=crbin2=255;
  
  gainit=0;
  ninst=0;

  while(quitf)
    {
      for(i=0;i<11000;i++)
        {
          exe6809();
          for(k=0;k<nbwait;k++);
          gainit++;
        }
      if (gainit>=6666) gainit-=6666;
      
      if ((++j)==3)
        {
          if (!(ccrest&0x10))
            {
              ccrest|=0x80;
              pshsr(0xff);
              ccrest|=0x10;
              pc=((buf[0xfff8]<<8)+(buf[0xfff9]&255))&0xffff;
            }
          j=0;
		}
      refreshv();
      while(XEventsQueued(display,QueuedAfterFlush))
        {
          XNextEvent(display,&ev);
          switch(ev.type) {
          case Expose:
            while(XCheckTypedEvent(display,Expose,&ev));
            refreshsc();
            break;
          case ButtonPress:
            switch(ev.xbutton.button) {
            case 2:
              crayx=(ev.xbutton.x-20)/2;
              crayy=(ev.xbutton.y-20)/2;
              break;
            case 1:
              buf[0xe7c3]|=2;
              break;
            case 3:
              quitf=0;
              break;
            default:
              break;	}
            break;
          case MotionNotify:
            crayx=(ev.xbutton.x-20)/2;
            crayy=(ev.xbutton.y-20)/2;
            break;
          case ButtonRelease:
            switch(ev.xbutton.button) {
            case 2:
              crayx=-1;
              break;
            case 1:
              buf[0xe7c3]&=0xfd;
              break;
            default:
              break;	}
            break;
          case KeyPress:
            key=ev.xkey.keycode&255;
            if (lkey[key])
              {
                i=0;
                while((i<nkey)&&(tch[i]!=key)) i++;
                if (i==nkey)
                  {
                    i=0;
                    while((i<nkey)&&(tch[i])) i++;
                    if (i==nkey) i=0;
                    tch[i]=key;
                  }
              }
            else if ((i=ckey[key])!=255)
              {
                if (i==0xfe)
                  {
                    pbin2&=0xbf;
                    crain2&=0x7f;
                  }
                if (i==0xfd)
                  {
                    pbin2&=0x7f;
                    crbin2&=0x7f;
                  }
                if (i<0xfd) pain2&=~i;
              }
            break;
          case KeyRelease:
            key=ev.xkey.keycode&255;
            if (lkey[key])
              {
                for(i=0;i<nkey;i++) 
                  if (tch[i]==key) tch[i]=0;
              }
            else	
              if ((i=ckey[key])!=255)
                {
                  if (i<0xfd) pain2|=i;
                  if (i==0xfe) 	
                    {
                      pbin2|=0x40;
                      crain2|=0x80;
                    }
                  if (i==0xfd)
                    {
                      pbin2|=0x80;
                      crbin2|=0x80;
                    }
                }
            break;
          default :
            break;	}
        }
    }
  return ninst;
}

