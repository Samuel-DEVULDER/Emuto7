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

  video_ami.c : routines video de l'emulateur (+gestion memoire)

  Adaptation pour l'Amiga par Samuel Devulder en 07/97.

*/

#include "emuto7.h"

#include <time.h>
#include <signal.h>

#include <exec/memory.h>
#include <exec/devices.h>
#include <exec/io.h>
#include <dos/dos.h>
#include <devices/timer.h>
#include <devices/audio.h>

#ifdef __GNUC__
#include <proto/alib.h>
#endif
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/graphics.h>

#include "monitor.h"
#include "config.h"

#define nbtouche 68
#define nkey 3

/* SAM: definir ceci pour utiliser le vieux code avec le buffer de
modification. (plus fluide et rapide sans modif avec gcc).
#define USE_MODIF
*/
/* SAM: definir ceci pour emuler le retour de 0x5E quand on lit dans
de la ram inexistante (c'est + rapide sans 0x5E)
#define USE_5E
*/
/* SAM: definir ceci si on prefere les switch()s (41% avec vs 33% sans
les switchs et gcc sur 030@40Mhz)
*/
#define USE_SWITCH

#ifndef __inline__
#define __inline__
#endif

static UBYTE codel[]=
{0x01,0x01,0x02,0x02,0x02,0x02,0x02,0x02,0x04,0x08,0x10,0x20,0x40,0x80
   ,0x04,0x08,0x10,0x20,0x40,0x80,0x80,0x40,0x20,0x10
     ,0x04,0x08,0x10,0x20,0x40,0x80,0x80,0x40,0x20,0x10
       ,0x04,0x08,0x10,0x20,0x40,0x80,0x80,0x40,0x20,0x10
         ,0x02,0x04,0x02,0x04,0x08,0x80
           ,0x04,0x08,0x04,0x08,0x10,0x20,0x40,0x08
             ,0x00,0x00,0x00,0x00,0x00
               ,0x00,0x00,0x00,0x00,0x00
};
static UBYTE codec[]=
{0x01,0x01,0x80,0x40,0x20,0x10,0x08,0x02,0x02,0x02,0x02,0x02,0x02,0x02
   ,0x80,0x80,0x80,0x80,0x80,0x80,0x40,0x40,0x40,0x40
     ,0x20,0x20,0x20,0x20,0x20,0x20,0x10,0x10,0x10,0x10
       ,0x08,0x08,0x08,0x08,0x08,0x08,0x04,0x04,0x04,0x04
         ,0x01,0x01,0x04,0x04,0x04,0x01
           ,0x40,0x40,0x10,0x10,0x01,0x01,0x01,0x01
             ,0xfe,0xfb,0xfd,0xf7,0x1
               ,0xef,0xbf,0xdf,0x7f,0x2
};

struct IntuitionBase    *IntuitionBase;
struct GfxBase          *GfxBase;

static struct Screen    *S;
static struct Window    *W;

static UBYTE decode[256][4];
static USHORT pal[] = {
    0xAAA,0xFAA,0xAFA,0xFFA,0xAAF,0xFAF,0xAFF,0xFA0,
    0x000,0xF00,0x0F0,0xFF0,0x00F,0xF0F,0x0FF,0xFFF
};

static UBYTE whichchannel[]={1,2,4,8};
static struct IOAudio *AudioIO;
static struct MsgPort *AudioMP;
/*static struct Message *AudioMSG;*/
static UBYTE  iosent,devopen;
static UWORD  *audiobuf;

static UBYTE   modif[8192];

static UBYTE   tch[nkey];
static UBYTE   lkey[256];
static UBYTE   ckey[256];
static FILE    *fl;
FILE    *fk7;
int     crayx,crayy;

int nbwait;

static short gainit;
UBYTE pa1,pb1;

UBYTE pain1,paout1,crain1;
UBYTE pbin1,pbout1,crbin1;

UBYTE pain2,paout2,crain2;
UBYTE pbin2,pbout2,crbin2;

#if __ASMSASC
extern __asm RefreshvASM(register __a0 int *,register __a1 int *,register __a2 int *,register __a3 int *);
extern __asm SubStoclASM(register __d0 long,register __d1 long,register __a0 unsigned char *,register __a1 unsigned char *);
extern __asm SubStocsASM(register __d0 long,register __d1 long,register __a0 unsigned char *,register __a1 unsigned char *);
extern long __asm SubLoadlASM(register __d1 long,register __a0 unsigned char *);
extern long __asm SubLoadsASM(register __d1 long,register __a0 unsigned char *);
#endif

void inicray(void)
{
  crayx=-1;
}

void inikey(char *name)
{
  int   i,key;

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

void inik7(void)
{
  fk7=NULL;
}

void gfxleave(void)
{
    if(iosent) {AbortIO((void*)AudioIO);iosent   = 0;}
    if(devopen) {CloseDevice((void*)AudioIO);devopen = 0;}
    if(AudioIO) {DeleteExtIO((void*)AudioIO);AudioIO = NULL;}
    if(AudioMP) {DeleteMsgPort((void*)AudioMP);AudioMP  = NULL;}
    if(audiobuf) {FreeMem((APTR)audiobuf,2);audiobuf = 0;}
    if(W) {CloseWindow((void*)W);W = NULL;}
    if(S) {CloseScreen((void*)S);S = NULL;}
    if(IntuitionBase) {CloseLibrary((void*)IntuitionBase); IntuitionBase = NULL;}
    if(GfxBase) {CloseLibrary((void*)GfxBase); GfxBase = NULL;}
}

void inisound(void)
{
    iosent = 0;
    AudioMP = (void*)CreateMsgPort();
    if(!AudioMP) return;
    AudioIO = (void *)CreateExtIO(AudioMP,sizeof(struct IOAudio));
    if(!AudioIO) return;

    devopen = 0;
    AudioIO->ioa_Request.io_Message.mn_Node.ln_Pri = 127;
    AudioIO->ioa_Data = whichchannel;
    AudioIO->ioa_Length = sizeof(whichchannel);
    AudioIO->ioa_AllocKey = 0;
    if(OpenDevice(AUDIONAME, 0, (void*)AudioIO, 0)) return;
    devopen = 1;

    audiobuf = (void *)AllocMem(2,MEMF_CHIP|MEMF_CLEAR);

    if(audiobuf) {
        AudioIO->ioa_Request.io_Command = CMD_WRITE;
        AudioIO->ioa_Request.io_Flags   = ADIOF_PERVOL|IOF_QUICK;
        AudioIO->ioa_Data               = (void*)audiobuf;
        AudioIO->ioa_Length             = 2;
        AudioIO->ioa_Period             = 500;
        AudioIO->ioa_Volume             = 64;
        AudioIO->ioa_Cycles             = 0;
        BeginIO((void*)AudioIO);
        iosent = 1;
    }
}

void iniscreen(char *name)
{
    int   i,j,k; /* l,n; */
    static struct NewScreen NewScreenStructure = {
        0,0, 320,200, 5, 0,1,
        0, CUSTOMSCREEN|SCREENQUIET|SCREENBEHIND,
        NULL, (void*)"UAE", NULL, NULL};
    static struct NewWindow NewWindowStructure = {
        0,0, 320,200, 0,1,
        IDCMP_MOUSEBUTTONS|IDCMP_RAWKEY|IDCMP_MOUSEMOVE,
        WFLG_BACKDROP|WFLG_RMBTRAP|WFLG_NOCAREREFRESH|
        WFLG_BORDERLESS|WFLG_WINDOWACTIVE|WFLG_REPORTMOUSE,
        NULL, NULL, (void*)"UAE", NULL, NULL, 5,5, 800,600,
        CUSTOMSCREEN};

    atexit(gfxleave);

    IntuitionBase = (void*)OpenLibrary("intuition.library",0);
    if(!IntuitionBase) {
        fprintf(stderr,"No intuition ?\n");
        exit(0);
    }

    GfxBase = (void*)OpenLibrary("graphics.library",0);
    if(!GfxBase) {
        fprintf(stderr,"No gfx ?\n");
        exit(0);
    }

    S = (void*)OpenScreen(&NewScreenStructure);
    if(!S) {
        fprintf(stderr,"No screen ?\n");
        exit(0);
    }

    NewWindowStructure.Width  = S->Width;
    NewWindowStructure.Height = S->Height;
    NewWindowStructure.Screen = S;
    W = (void*)OpenWindow(&NewWindowStructure);
    if(!W) {
        fprintf(stderr,"No window ?\n");
        exit(0);
    }

    for(i=0;i<16;++i) SetRGB4(&S->ViewPort,i,0,0,0);
    for(i=0;i<16;++i) SetRGB4(&S->ViewPort,i+16,pal[i]>>8,(pal[i]>>4)&15,pal[i]&15);

    SetAPen(&S->RastPort, 16);
    RectFill(&S->RastPort,0,0,319,199);

    for(j=0;j<16;++j) for(i=0;i<16;++i) for(k=0;k<4;++k) {
        decode[j*8+(i&7)+(i&8)*16][k] =
            (((j>>k)&1)<<1) | ((i>>k)&1);
    }
#if 0
    /*---------------------*/
    {
	USHORT fond;
	for(fond=0;fond<256;++fond) {
	    UBYTE fd,fr;
	    USHORT k;
	    fr = (fond>>3)&15;
	    fd = (fond&7)|((fond&128)>>4);
	    for(k=0;k<4;++k) {
		USHORT fo;
		switch((((fd>>k)&1)<<1)|((i>>k)&1)) {
		case 0: 
		    for(fo=0;fo<256;++fo) forme_fond[fo][fond][k]=0x00;
		    break;
		case 1:
		    for(fo=0;fo<256;++fo) forme_fond[fo][fond][k]=~fo;
		    break;
		case 2:
		    for(fo=0;fo<256;++fo) forme_fond[fo][fond][k]=fo;
		    break;
		case 3:
		    for(fo=0;fo<256;++fo) forme_fond[fo][fond][k]=0xFF;
		    break;
		}
	    }
	}
    }
    /*---------------------*/
#endif
    inikey(name);
    inicray();
    inik7();

    inisound();

    i = 0;
    j = clock();while(clock()==j);
    j = clock();while(clock()-j<CLOCKS_PER_SEC) ++i;
    j = ((35*i)/22000)-21;
    if(j>0) {
        nbwait = j;
        fprintf(stderr,"default speed = %d\n",j);
        fflush(stderr);
    }
}

void refreshv(void)
{
    unsigned char **bm  = &S->RastPort.BitMap->Planes[0];
    unsigned char  *fr  = &buf[0x04000];
    unsigned char  *fd  = &buf[0x10000];
    unsigned char  *mod = modif;
    short i;
/*    static char frame=1; if(--frame) return; else frame=1; /**/

#if defined(__ASMSASC)
    RefreshvASM((int *)&buf[0x04000],(int *)modif,(int *)decode,(int *)bm);
#else

/*    WaitBOVP(&S->ViewPort);*/
    for(i=0;i<8000;++i) if(*mod++) {
        unsigned char *dec = decode[*fd++];
        unsigned char cmd, frm = *fr++;
        short j;
        mod[-1] = 0;
        for(j=4;j--;) {
            if(!(cmd=*dec++)) (*bm++)[i] = 0x00;
            else if(!--cmd)   (*bm++)[i] = ~frm;
            else if(!--cmd)   (*bm++)[i] = frm;
            else              (*bm++)[i] = 0xFF;
        }
        bm -= 4;
    } else fd++,fr++;
#endif
}

void refreshsc(void)
{
  int   i;
#ifdef USE_MODIF
  for(i=0;i<8192;i++) modif[i]=1;
  refreshv();
#endif
}

static void __inline__ tour(UBYTE cl)
{
    static UBYTE _cl=255;
    if((cl&0x74)==_cl) return;
    _cl=cl&0x74;
    cl=((cl&0x70)>>4)+((cl&4)<<1);
    SetRGB4(&S->ViewPort,0,pal[cl]>>8,(pal[cl]>>4)&15,pal[cl]&15);
}

static void __inline__ updt_scr(int k)
{
    unsigned char **bm, *dec, frm, cmd;
    short i;
    if((USHORT)k>=(USHORT)0x5f40) return;
    bm  = &S->RastPort.BitMap->Planes[0];
    frm = buf[k];
    dec = decode[(unsigned char)buf[k+0xc000]];
    k-=0x4000;
    for(i=4;i--;) {
	if(!(cmd=*dec++)) (*bm++)[k] = 0x00;
        else if(!--cmd)   (*bm++)[k] = ~frm;
        else if(!--cmd)   (*bm++)[k] = frm;
        else              (*bm++)[k] = 0xFF;
    }
}

static void __inline__ updt_scrl(int k)
{
    unsigned char **bm, *dec1,*dec2, frm1, frm2, cmd;
    short i;
    if((USHORT)k>=(USHORT)0x5f40) return;
    bm   = &S->RastPort.BitMap->Planes[0];
    dec1 = decode[(unsigned char)buf[k+0xc000]];
    dec2 = decode[(unsigned char)buf[k+0xc001]];
    frm1 = buf[k]; 
    frm2 = buf[k+1];
    k-=0x4000;
    for(i=4;i--;) {
	if(!(cmd=*dec1++)) (*bm)[k] = 0x00;
        else if(!--cmd)    (*bm)[k] = ~frm1;
        else if(!--cmd)    (*bm)[k] = frm1;
        else               (*bm)[k] = 0xFF;
	if(!(cmd=*dec2++)) (*bm++)[k+1] = 0x00;
        else if(!--cmd)    (*bm++)[k+1] = ~frm2;
        else if(!--cmd)    (*bm++)[k+1] = frm2;
        else               (*bm++)[k+1] = 0xFF;
    }
}

static void __inline__ set_e7c0(USHORT k, UBYTE val)
{
    switch(k&15) {
       case 0x0: break;
       case 0x1: if(iosent) *audiobuf = (val&8)?0x8080:0x7F7F; break;
       case 0x2: break;
       case 0x3: ((short*)&offset_fond)[1] = (val&1)?0:0xc000; 
	         val |= 128; tour(val); break;
       case 0x4: case 0x5: case 0x6: case 0x7: break;
       case 0x8: if(buf[0xe7ca]&4) {paout1=val;return;} break;
       case 0x9: if(buf[0xe7cb]&4) {pbout1=val;return;} break;
       case 0xa: case 0xb: break;
       case 0xc: if(buf[0xe7ce]&4) {paout2=val;return;} break;
       case 0xd: if(buf[0xe7cf]&4) {pbout2=val;if(iosent) 
				    *audiobuf=(val&63)*0x202;return;} break;
       case 0xe: break;
       case 0xf: break;
    }
    buf[k]=val;
}

static long __inline__ get_e7c0(USHORT k)
{
  switch(k&0xFF) {
      case 0xc0: return 0x81;
      case 0xc8:
          if(buf[0xe7ca]&4) {
              USHORT i,pb=(0x80>>(pbout1&7));
	      UBYTE pain1_=0xff;
              for(i=nkey;i--;)
                pain1_&=((pb&lkey[tch[i]])?ckey[tch[i]]:0xff);
	      pain1=pain1_;
              return ((paout1&buf[0xe7c8])+(pain1&(~buf[0xe7c8])))&255;
          } break;
      case 0xc9: if(buf[0xe7cb]&4)
            return ((pbout1&buf[0xe7c9])+(pbin1&(~buf[0xe7c9])))&255;
            break;
      case 0xca: return (buf[0xe7ca]&0x7f)+(crain1&0x80);
      case 0xcb: return (buf[0xe7cb]&0x7f)+(crbin1&0x80);
      case 0xcc: if(buf[0xe7ce]&4)
            return ((paout2&buf[0xe7cc])+(pain2&(~buf[0xe7cc])))&255;
            break;
      case 0xcd: if(buf[0xe7cf]&4)
            return ((pbout2&buf[0xe7cd])+(pbin2&(~buf[0xe7cd])))&255;
            break;
      case 0xce: return (buf[0xe7ce]&0x7f)|(crain2&0x80);
      case 0xcf: return (buf[0xe7cf]&0x7f)|(crbin2&0x80);
      case 0xe7:
          if (gainit>=6666) gainit-=6666;
          if (gainit>=4266) return 0;
          return 128;
      default: break;
      }
      return buf[k];
}

long REGPARM loads(long k)
{
  unsigned char *BUF=buf;

#if (defined(__ASMSASC) && defined(USE_MODIF))
  if((USHORT)k<(USHORT)0xe000) return SubLoadsASM(k,BUF); 
#endif
#ifndef USE_SWITCH
  if((USHORT)k < (USHORT)0x4000) {
  } else if((USHORT)k < (USHORT)0x6000) { 
      k+=offset_fond; 
  } else if((USHORT)k < (USHORT)0xA000) {
  } else if((USHORT)k < (USHORT)0xE000) { 
      if(buf[0xe7c9]&0x10) k+=0x8000; 
#ifdef USE_5E
      else if(!(buf[0xe7c9]&8)) return 0x5e;
#endif
  } else if(short_msk(k,0xFFC0,0xE7C0)) return get_e7c0(k);
  return BUF[k];
#else
  switch(((USHORT)k)>>12) {
  case 0x0: case 0x1: case 0x2: case 0x3: break;
  case 0x4: case 0x5: 
      k+=offset_fond;
      break;
  case 0xa: case 0xb: case 0xc: case 0xd:
      if( buf[0xe7c9]&0x10) k+=0x8000; else 
      if((buf[0xe7c9]&0x08)==0) return 0x5E;
      break;
  case 0xe: 
      if(short_msk(k,0xFFC0,0xE7C0)) return get_e7c0(k);
      break;
  }
  return BUF[k];
#endif
}

long REGPARM loadl(long k)
{
  unsigned char *BUF=buf;

#ifdef __ASMSASC
  if((USHORT)k<(USHORT)0xe000) return SubLoadlASM(k,BUF);
#endif

#ifndef USE_SWITCH
  if((USHORT)k < (USHORT)0x4000) {
  } else if((USHORT)k < (USHORT)0x6000) { 
      k+=offset_fond; 
  } else if((USHORT)k < (USHORT)0xA000) {
  } else if((USHORT)k < (USHORT)0xE000) { 
      if(buf[0xe7c9]&0x10) k+=0x8000; 
#ifdef USE_5E
      else if(!(buf[0xe7c9]&8)) return 0x5E5E;
#endif
  } else if(short_msk(k,0xFFC0,0xE7C0)) return (get_e7c0(k)<<8)|get_e7c0(k+1);
  return get_short(BUF+k);
#else
  switch(((USHORT)k)>>12) {
  case 0: case 1: case 2: case 3: break;
  case 0x4: case 0x5:
      k += offset_fond; 
      break;
  case 0xa: case 0xb: case 0xc: case 0xd:
      if( buf[0xe7c9]&0x10) k+=0x8000; else
      if((buf[0xe7c9]&0x08)==0) return 0x5e5e;
      break;
  case 0xe:
      if(short_msk(k,0xFFC0,0xE7C0)) return (get_e7c0(k)<<8)|get_e7c0(k+1);
      break;
  }
  return get_short(BUF+k);
#endif
}

#ifdef USE_MODIF
#define MODIF_CHOICE(X,Y) X
#else
#define MODIF_CHOICE(X,Y) Y
#endif

int REGPARM stocs(long k,long val)
{
  unsigned char *BUF=buf;
#if defined(__ASMSASC)&&defined(USE_MODIF)
  if ((USHORT)k<(USHORT)0xe000) {SubStocsASM(k,val,BUF,modif);return 0;}
#endif

#ifndef USE_SWITCH
  if ((USHORT)k<(USHORT)0x4000) {
      /* rom: on fait rien */
  } else if ((USHORT)k<(USHORT)0x6000) {
      BUF += k; BUF += offset_fond; 
      MODIF_CHOICE({modif[k-0x4000] |= BUF[0]^val; BUF[0]=val;},
		   {BUF[0]=val; updt_scr(k);})
  } else if ((USHORT)k<(USHORT)0xA000) {
      BUF[k]=val;
  } else if ((USHORT)k<(USHORT)0xE000) {
      if(buf[0xe7c9]&0x10) k += 0x8000; 
      if(buf[0xe7c9]&0x18) BUF[k] = val;
  } else if(short_msk(k,0xFFC0,0xE7C0)) set_e7c0(k,val);
  return 0;
#else
  switch(((USHORT)k)>>12) {
  case 0x0: case 0x1: case 0x2: case 0x3: break;
  case 0x4: case 0x5:
      BUF += k; BUF += offset_fond; 
      MODIF_CHOICE({modif[k-0x4000] |= BUF[0]^val; BUF[0]=val;},
		   {BUF[0]=val; updt_scr(k);})
      break;
  case 0x6: case 0x7: case 0x8: case 0x9:
      BUF[k]=val;
      break;
  case 0xa: case 0xb: case 0xc: case 0xd:
      if(buf[0xe7c9]&0x10) k += 0x8000; 
      if(buf[0xe7c9]&0x18) BUF[k] = val;
      break;
  case 0xe:
      if(short_msk(k,0xFFC0,0xE7C0)) set_e7c0(k,val);
      break;
  }
  return 0;
#endif
}

int REGPARM stocl(long k,long val)
{
  unsigned char *BUF=buf;
#if defined(__ASMSASC)&&defined(USE_MODIF)
  if ((USHORT)k<(USHORT)0xe000) {SubStoclASM(k,val,BUF,modif);return 0;}
#endif

#ifndef USE_SWITCH
  if ((USHORT)k<(USHORT)0x4000) {
      /* rom: on fait rien */
  } else if ((USHORT)k<(USHORT)0x6000) {
      BUF += k; BUF += offset_fond; 
      MODIF_CHOICE({
	  *(short*)&modif[k-0x4000] |= (*(short*)&BUF[0])^val;
	  *(short*)&BUF[0]=val; 
      },{ *(short*)&BUF[0]=val; 
	  updt_scrl(k);
      })
  } else if ((USHORT)k<(USHORT)0xA000) {
      *(short*)&BUF[k]=val;
  } else if ((USHORT)k<(USHORT)0xE000) {
      if(buf[0xe7c9]&0x10) k += 0x8000; 
      if(buf[0xe7c9]&0x18) *(short*)&BUF[k] = val;
  } else if(short_msk(k,0xFFC0,0xE7C0)) {set_e7c0(k,val>>8);set_e7c0(k+1,val);}
  return 0;
#else
  switch(((USHORT)k)>>12) {
  case 0x0: case 0x1: case 0x2: case 0x3: break;
  case 0x4: case 0x5: 
      BUF += k; BUF += offset_fond; 
      MODIF_CHOICE({
	  *(short*)&modif[k-0x4000] |= (*(short*)&BUF[0])^val;
	  *(short*)&BUF[0]=val;
      },{ *(short*)&BUF[0]=val; 
	  updt_scrl(k);
      })
      break;
  case 0x6: case 0x7: case 0x8: case 0x9: 
      *(short*)&BUF[k]=val;
      break;
  case 0xa: case 0xb: case 0xc: case 0xd:
      if(buf[0xe7c9]&0x10) k += 0x8000;
      if(buf[0xe7c9]&0x18) *(short*)&BUF[k]=val;
      break;
  case 0xe: 
     if(short_msk(k,0xFFC0,0xE7C0)) {set_e7c0(k,val>>8);set_e7c0(k+1,val);}
     break;
  }
  return 0;
#endif
}

long    compt;
int     quitf;

int bouclex(void)
{
  struct IntuiMessage *msg;
  short i;
  short j;
  int   k;
  int   key;

  j=0;
  compt=0;
  for(i=nkey;i--;) tch[i]=0;
  quitf=1;

  pain2=paout2=crain2=255;
  pbin2=pbout2=crbin2=255;

  gainit=0;

  ScreenToFront(W->WScreen);

  while(quitf)
    {
      if(nbwait)
      for(i=0;i<11000;i++) 
        {
          exe6809();
          for(k=0;k<nbwait;k++);
          gainit++;
        }
      else for(i=11000;i--;) {exe6809();++gainit;}
      
      if (gainit>=6666) gainit-=6666;

      if ((++j)==3)
        {
        static clock_t lastt,lasttt,cpt = 10;
        if(lastt) {
	   clock_t t = clock();
	   int r;
	   if(!nbwait && ((t-lastt)*10<CLOCKS_PER_SEC)) nbwait = 1;
	   r = (nbwait*(CLOCKS_PER_SEC+1))/(10*(t-lastt)+1);
	   if(r!=nbwait) {
/*	      fprintf(stderr,"New speed=%d\r",r);
	      fflush(stderr);
*/	      nbwait = r;
	   }
           if(!--cpt) {
	      cpt = 10;
              if(t-lasttt)
	      fprintf(stderr,"Speed=%d%%   \r",
	             (100*cpt*CLOCKS_PER_SEC)/(10*(t-lasttt)));
	      else fprintf(stderr,"Speed=N/A    \r");
	      fflush(stderr);
              lasttt = t;
	    }
            lastt = t;
	  } else lastt = clock();

          if (!(ccrest&0x10))
            {
              ccrest|=0x80;
              pshsr(0xff);
              ccrest|=0x10;
              pc=get_short(buf+0xFFF8);
            }
          j=0;
        }
#ifdef USE_MODIF
      refreshv();
#endif
      if(SetSignal(0L, SIGBREAKF_CTRL_C|SIGBREAKF_CTRL_D) &
                    (SIGBREAKF_CTRL_C|SIGBREAKF_CTRL_D)) {
         gfxleave(); /* sam: for SAS/C */
         raise(SIGINT);
      }
      while(msg=(void*)GetMsg(W->UserPort))
        {
            int x,y,class,code;
            class = msg->Class;
            code  = msg->Code;
            x     = msg->MouseX;
            y     = msg->MouseY;
            ReplyMsg((struct Message*)msg);

            switch(class) {
            case IDCMP_MOUSEBUTTONS:
                crayx = x;
                crayy = y;
                if(code==SELECTDOWN) buf[0xE7C3] |= 2;
                if(code==SELECTUP)   buf[0xE7C3] &=~2;
                if(code==MENUDOWN)   quitf=0;
                if(code==MENUUP)     quitf=1;
            break;

            case IDCMP_MOUSEMOVE:
                crayx = x;
                crayy = y;
            break;

            case IDCMP_RAWKEY:
                key = code&127;
                if(code&128) { /* release */
                    if (lkey[key]) {
                        for(i=0;i<nkey;i++) if (tch[i]==key) tch[i]=0;
                    } else if ((i=ckey[key])!=255) {
                        if (i<0xfd) pain2|=i;
                        if (i==0xfe) {pbin2|=0x40;crain2|=0x80;}
                        if (i==0xfd) {pbin2|=0x80;crbin2|=0x80;}
                    }
                } else {       /* press */
                    if (lkey[key]) {
                        i=0;
                        while((i<nkey)&&(tch[i]!=key)) i++;
                        if (i==nkey) {
                            i=0;
                            while((i<nkey)&&(tch[i])) i++;
                            if (i==nkey) i=0;
                            tch[i]=key;
                        }
                    } else if ((i=ckey[key])!=255) {
                        if (i==0xfe) {pbin2&=0xbf;crain2&=0x7f;}
                        if (i==0xfd) {pbin2&=0x7f;crbin2&=0x7f;}
                        if (i<0xfd) pain2&=~i;
                    }
                }
            break;
            }
        }
    }
  fprintf(stderr,"                      \r");
  fflush(stderr);
  ScreenToBack(W->WScreen);

  return 0;
}

