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

 emup1.c : emulateur 6809. 1ere partie (+io to7)

*/

#include "emuto7.h"
#include "monitor.h"
#include "video.h"

extern int quitf;

/*
#ifdef AMIGA
#define m1   ((short*)&m1)[1]
#define m2   ((short*)&m2)[1]
#define ovfl ((short*)&ovfl)[1]
#define sign ((short*)&sign)[1]
#define res  ((short*)&res)[1]
#endif
*/

void negm(void)         /* H?NxZxVxCx */
{
  long  k;
  int   val;

  ASMOPT("movel _ad,d0; lea _adresc,a0; movel a0@(d0:l:4),a0; jbsr a0@;"
         " movel d0,sp@-; jbsr _loads;"
         " moveb d0,_m1+3; negw d0; moveb d0,_m2+3;"
         " movew d0,_sign+2; movew d0,_ovfl+2; movew d0,_res+2;"
         " moveb d0,d1; movel sp@+,d0; jbsr _stocs");

  val=loads(k=(*adresc[ad])());
  m1=val; m2=-val;              /* bit V */
  stocs(k,val=-val);
  ovfl=sign=res=val;
}

void comm(void)         /* NxZxV0C1 */
{
  long  k;
  int   val;

  ASMOPT("moveb _ovfl+3,_m1+3; "\
         " movel _ad,d0; lea _adresc,a0; movel a0@(d0:l:4),a0; jbsr a0@;"
         " movel d0,sp@-; jbsr _loads;"
         " notb d0; orw #256,d0; moveb d0,_sign+3; movew d0,_res+2;"
         " moveb d0,d1; movel sp@+,d0; jbsr _stocs");

  val=loads(k=(*adresc[ad])());
  /*m1=~m2;*/m1=ovfl;
  stocs(k,val=~val);
  sign=val;
  res=0x100|(val&255);
}

void lsrm(void)         /* N0ZxCx */
{
  long  k;
  int   val;

  ASMOPT("movel _ad,d0; lea _adresc,a0; movel a0@(d0:l:4),a0; jbsr a0@; "
         "movel d0,sp@-; jbsr _loads; clrb _sign+3; "
         "moveb d0,_res+2; lsrb #1,d0; moveb d0,d1; moveb d0,_res+3; "
         "movel sp@+,d0; jbsr _stocs");

  val=loads(k=(*adresc[ad])());
/*  res=(val&1)<<8;       /* bit C */
  res = val<<8;           /* sam: bit C */
  stocs(k,val>>=1);
  sign=0;
  res&=~255;
  res|=(val&255);
}

void rorm(void)         /* NxZxCx */
{
  long  k;
  int   val, i;

  ASMOPT("movel _ad,d0; lea _adresc,a0; movel a0@(d0:l:4),a0; jbsr a0@; "
         "movel d0,sp@-; jbsr _loads; "
         "movew _res+2,d1; moveb d0,_res+2; moveb d0,d1; "
         "lsrw #1,d1; moveb d1,_res+3; moveb d1,_sign+3; "
         "movel sp@+,d0; jbsr _stocs");

  i=val=loads(k=(*adresc[ad])());
  val |= (res&0x100);
  val >>= 1;
  stocs(k,val);
  sign=val;
  i <<= 8; 
  res=i|(val&255);
}

void asrm(void)         /* H?NxZxCx */
{
  long  k;
  int   val;

  ASMOPT("movel _ad,d0; lea _adresc,a0; movel a0@(d0:l:4),a0; jbsr a0@; "
         "movel d0,sp@-; jbsr _loads; moveb d0,_res+2; asrb #1,d0; "
	 "moveb d0,_res+3; moveb d0,_sign+3; moveb d0,d1; "
	 "movel sp@+,d0; jbsr _stocs");

  val   = loads(k=(*adresc[ad])());
  res   = val<<8;
  stocs(k,val=((val&255)>>1)|(val&0x80));
  sign=val;
  res&=~255;
  res|=val&255;
}

void aslm(void)         /* H?NxZxVxCx */
{
  long  k;
  int   val;

  ASMOPT("movel _ad,d0; lea _adresc,a0; movel a0@(d0:l:4),a0; jbsr a0@; "
         "movel d0,sp@-; jbsr _loads; "
         "moveb d0,_m1+3; moveb d0,_m2+3; addw d0,d0; "
         "moveb d0,_sign+3; moveb d0,_ovfl+3; movew d0,_res+2; "
         "moveb d0,d1; movel sp@+,d0; jbsr _stocs");

  val=loads(k=(*adresc[ad])());
  m1=m2=val;
  stocs(k,val<<=1);
  ovfl=sign=res=val;
}

void rolm(void)         /* NxZxVxCx */
{
  long  k;
  int   val,i;

  ASMOPT("movel _ad,d0; lea _adresc,a0; movel a0@(d0:l:4),a0; jbsr a0@;"
         " movel d0,sp@-; jbsr _loads;"
         " moveb d0,_m1+3; moveb d0,_m2+3; moveq #1,d1; andb _res+2,d1; "
         " addw d0,d0; orw d0,d1; "
         " moveb d1,_sign+3; moveb d1,_ovfl+3; movew d1,_res+2; "
         " movel sp@+,d0; jbsr _stocs");

  i=val=loads(k=(*adresc[ad])());
  m1=m2=val;
  stocs(k,val=(val<<1)|((res&0x100)>>8) );
  ovfl=sign=res=val;
}

void decm(void)         /* NxZxVx */
{
  long  k;
  int   val;

  ASMOPT("movel _ad,d0; lea _adresc,a0; movel a0@(d0:l:4),a0; jbsr a0@;"
         " movel d0,sp@-; jbsr _loads;"
         " moveb d0,_m1+3; moveb #128,_m2+3;"
         " subqb #1,d0;"
         " moveb d0,_sign+3; moveb d0,_ovfl+3; moveb d0,_res+3;"
         " moveb d0,d1; movel sp@+,d0; jbsr _stocs");
  val=loads(k=(*adresc[ad])());
  m1=val; m2=0x80;
  add_modulo(char,val,-1);
  stocs(k,val);
  ovfl=sign=val/*&255*/;
/*  res=(res&0x100)|sign;*/
  res&=~255;
  res|=(val&255);
}

void incm(void)         /* NxZxVx */
{
  long  k;
  int   val;

  ASMOPT("movel _ad,d0; lea _adresc,a0; movel a0@(d0:l:4),a0; jbsr a0@;"
         " movel d0,sp@-; jbsr _loads;"
         " moveb d0,_m1+3; clrb _m2+3;"
         " addqb #1,d0;"
         " moveb d0,_sign+3; moveb d0,_ovfl+3; moveb d0,_res+3;"
         " moveb d0,d1; movel sp@+,d0; jbsr _stocs");

  val=loads(k=(*adresc[ad])());
  m1=val; m2=0;
  add_modulo(char,val,1);
  stocs(k,val);
  ovfl=sign=val/*&255*/;
/*  res =(res&0x100)|sign;*/
  res&=~255;
  res|=val&255;
}

void tstm(void)         /* NxZxV0 */
{
  int   val;

  ASMOPT("moveb _ovfl+3,_m1+3; "\
         " movel _ad,d0; lea _adresc,a0; movel a0@(d0:l:4),a0; jbsr a0@;"
         " jbsr _loads;"
         " moveb d0,_sign+3; moveb d0,_res+3;");

  val=loads((*adresc[ad])());
/*  m1=~m2;*/ m1=ovfl;
  sign=val;
  res&=~255;
  res|=val&255;
}

long posit(void)
{
  if (fk7) return ftell(fk7);
  return 0;
}

void iosb(void)
{
  if ((short)pc==(short)0xfbd4)       /*      crayon optique  */
    {
      res&=0xff;
      if (crayx==-1)
        res|=0x100;
      else
        {
          xr=crayx%320;
          yr=crayy%200;
        }
    }
  else if ((short)pc==(short)0xf07d)  /*      k7      */
    {
      int       i;

      res&=0xff;
      switch(buf[0x6029]&15)    {
      case 1:
        printf("\n");
        printhx(posit(),4);
        printf(" : ouverture lecture\n");
        buf[0x602a]=1;
        break;
      case 2:
        /* printf("*"); seg_/sreg: on vire ca pour charger plus vite */
        if (fk7) i=fgetc(fk7);
        else i=EOF;
        if (i==EOF)
          {
            res|=0x100;
            buf[0x602a]=-128; /* seg_/sreg: warn sas 0x80; */
          }
/*        if (ftell(fk7)==0x256) quitf=0;*/
        br=i&255;
        break;
      case 4:
        printf("\n");
        printhx(posit(),4);
        printf(" : ouverture ecriture\n");
        if ((fk7)&&(buf[0x602a]!=4))
          for(i=0;i<10;i++) fputc(0,fk7);
        buf[0x602a]=4;
        break;
      case 8:
        printf(".");
        if (fk7) fputc(br,fk7);
        break;
      default:
        printf("\n");
        printhx(posit(),4);
        printf(" : fermeture\n");
        buf[0x602a]=16;
        break;  }
    }
  else
    printf("erreur 01 pc=%x\n",pc);
}

void jmpm(void)
{
  pc=(*adresl[ad])();
}

static void jsr_(long val)
{
  long k;
  madd_modulo(short,sr,-2);
  k=pc;pc=val;
  stocl(sr,k);
}

void jsrm(void)
{
  long k;
  ASMOPT("movel _ad,d0; lea _adresl,a0; movel a0@(d0:l:4),a0; jbsr a0@; "
	 "movew _pc+2,d1; movew d0,_pc+2; movew _xyus+14,d0; subqw #2,d0; "
	 "movew d0,_xyus+14; jbsr _stocl;")
  jsr_((*adresl[ad])());
}
void JSRe(void)
{
  ASMOPT("movew a4@,d0; "
	 "movew _pc+2,d1; movew d0,_pc+2; movew _xyus+14,d0; subqw #2,d0; "
	 "movew d0,_xyus+14; jbsr _stocl;")
  jsr_(get_short(op));
}
void JSRx(void)
{
  ASMOPT("jbsr _indir; "
	 "movew _pc+2,d1; movew d0,_pc+2; movew _xyus+14,d0; subqw #2,d0; "
	 "movew d0,_xyus+14; jbsr _stocl;")
  jsr_(indir());
}

void clrm(void) /* N0Z1V0C0 */
{
  ASMOPT("moveb _ovfl+3,_m1+3; "\
         "movel _ad,d0; lea _adresc,a0; movel a0@(d0:l:4),a0; jbsr a0@; "
         "moveq #0,d1; moveb d1,_sign+3; movew d1,_res+2; jbsr _stocs");
  stocs((*adresc[ad])(),0);
  /*m1=~m2;*/ m1=ovfl;
  sign=res=0;
}

void nopm(void)
{
}

void synm(void)
{
  /*    printf("sync\n");
   */   
}

void lbra(void)
{
  madd_modulo(short, pc, get_short(op));
}

void lbsr(void)
{
  long k;
  madd_modulo(short, sr, -2);
  k=pc;lbra();
  stocl(sr,k);
}

void daam(void) /* NxZxV?Cx */
{
  int   i=ar+(res&0x100);
  if (((ar&15)>9)||((h1&15)+(h2&15)>15)) i+=6;
  if (i>0x99) i+=0x60;
  sign=res=i;
  ar=i&255;
  /*    printf("daa\n");
   */
}

void orcc(void)
{
  setcc(getcc()|(*op));
}

void andc(void)
{
  setcc(getcc()&(*op));
}

void sexm(void) /* NxZx */
{
  char b;

  ASMOPT("moveb _br+3,d1; smib d0; moveb d0,_ar+3; moveb d0,_sign+3; "
	 "moveb d1,_res+3");
/*
  ASMOPT("moveb _br+3,d0; extw d0; lsrw #8,d0;"
         " moveb d0,_ar+3; moveb d0, _sign+3; moveb d0,_res+3");
*/
  b=br;
  if(b<0) sign=ar=255; else sign=ar=0;
  res&=~255;
  res|=b&255;
}

void exgm(void)
{
  long  k,l;
  switch((0xF0&*op) >> 4) {
    case 0x0: k=(ar<<8)|br; break; case 0x1: k=xr; break;
    case 0x2: k=yr; break;         case 0x3: k=ur; break;
    case 0x4: k=sr; break;         case 0x5: k=pc; break;
    case 0x8: k=ar; break;         case 0x9: k=br; break;
    case 0xA: k=getcc(); break;    case 0xB: k=dp; break;
    default: k=0; break;
  }
  switch(*op & 15) {
    case 0x0: l=(ar<<8)|br; break; case 0x1: l=xr; break;
    case 0x2: l=yr; break;         case 0x3: l=ur; break;
    case 0x4: l=sr; break;         case 0x5: l=pc; break;
    case 0x8: l=ar; break;         case 0x9: l=br; break;
    case 0xA: l=getcc(); break;    case 0xB: l=dp; break;
    default: l=0; break;
  }
  switch((0xF0 & *op)>>4) {
    case 0x0: ar=l>>8;br=l&255; break; case 0x1: xr=l; break;
    case 0x2: yr=l; break;             case 0x3: ur=l; break;
    case 0x4: sr=l; break;             case 0x5: pc=l; break;
    case 0x8: ar=l; break;             case 0x9: br=l; break;
    case 0xA: setcc(l); break;         case 0xB: dp=l; break;
  }
  switch(0x0F & *op) {
    case 0x0: ar=k>>8;br=k&255; break; case 0x1: xr=k; break;
    case 0x2: yr=k; break;             case 0x3: ur=k; break;
    case 0x4: sr=k; break;             case 0x5: pc=k; break;
    case 0x8: ar=k; break;             case 0x9: br=k; break;
    case 0xA: setcc(k); break;         case 0xB: dp=k; break;
  }
/*
  int   o1,o2;
  long  *p,*q;

  o1=((*op)&0xf0)>>4;
  o2=(*op)&15;
  if (p=exreg[o1]) k=*p;
  else  if (o1) k=getcc();
  else k=(ar<<8)+br;
  if (q=exreg[o2])
    {
      l=*q;
      *q=k;
    }
  else  if (o2)
    {
      l=getcc();
      setcc(k);
    }
  else
    {
      l=(ar<<8)+br;
      ar=(k>>8)&255;
      br=k&255;
    }
  if (p) *p=l;
  else if (o1) setcc(l);
  else
    {
      ar=(l>>8)&255;
      br=l&255;
    }
*/
}

void tfrm(void)
{
  long  k;

  switch((0xF0 & *op)>>4) {
    case 0x0: k=(ar<<8)|br; break; case 0x1: k=xr; break;
    case 0x2: k=yr; break;         case 0x3: k=ur; break;
    case 0x4: k=sr; break;         case 0x5: k=pc; break;
    case 0x8: k=ar; break;         case 0x9: k=br; break;
    case 0xA: k=getcc(); break;    case 0xB: k=dp; break;
    default: k=0; break;
  }
  switch(0x0F & *op) {
    case 0x0: ar=k>>8;br=k&255; break; case 0x1: xr=k; break;
    case 0x2: yr=k; break;             case 0x3: ur=k; break;
    case 0x4: sr=k; break;             case 0x5: pc=k; break;
    case 0x8: ar=k; break;             case 0x9: br=k; break;
    case 0xA: setcc(k); break;         case 0xB: dp=k; break;
  }
/*
  int   o1,o2;
  long  *p,*q;

  o1=((*op)&0xf0)>>4;
  o2=(*op)&15;
  if (p=exreg[o1]) k=*p;
  else if (o1) k=getcc();
  else k=(ar<<8)+br;
  if (q=exreg[o2]) *q=k;
  else if (o2)  setcc(k);
  else
    {
      ar=(k>>8)&255;
      br=k&255;
    }
*/
}

void bras(void)
{
  madd_modulo(short, pc, (char)op[0]);
}
void brns(void)
{
}

void bhis(void) /* c|z=0 */
{
  if ((!(res&0x100))&&(res&0xff)) bras();
}
void blss(void) /* c|z=1 */
{
  if ((res&0x100)||(!(res&0xff)))       bras();
}

void bccs(void) /* c=0 */
{
  if (!(res&0x100)) bras();
}
void blos(void) /* c=1 */
{
  if (res&0x100) bras();
}

void bnes(void) /* z=0 */
{
  if (res&0xff) bras();
}
void beqs(void) /* z=1 */
{
  if (!(res&0xff)) bras();
}

void bvcs(void) /* v=0 */
{
  if ( ((m1^m2)&0x80)||(!((m1^ovfl)&0x80)) ) bras();
}
void bvss(void) /* v=1 */
{
  if ( (!((m1^m2)&0x80))&&((m1^ovfl)&0x80) ) bras();
}

void bpls(void) /* n=0 */
{
  if (!(sign&0x80)) bras();
}
void bmis(void) /* n=1 */
{
  if (sign&0x80) bras();
}

void bges(void) /* n^v=0 */
{
  if (!((sign^((~(m1^m2))&(m1^ovfl)))&0x80)) bras();
}
void blts(void) /* n^v=1 */
{
  if ((sign^((~(m1^m2))&(m1^ovfl)))&0x80) bras();
}

void bgts(void) /* z|(n^v)=0 */
{
  if ( (res&0xff)
      &&(!((sign^((~(m1^m2))&(m1^ovfl)))&0x80)) ) bras();
}
void bles(void) /* z|(n^v)=1 */
{
  if ( (!(res&0xff))
      ||((sign^((~(m1^m2))&(m1^ovfl)))&0x80) ) bras();
}
/*
#define LEA(a) \
  "movel _ad,d0; lea _adresc,a0; movel a0@(d0:l:4),a0; jbsr a0@; "\
  "movew d0,_"#a"+2"
*/
#define LEA(a) \
  "jbsr _indir; movew d0,_"#a"+2"
#define LEAX(a) LEA(a)"; sneb _res+3"
void leax(void) /* Zx */
{
  ASMOPT(LEAX(xyus));
/*
  if(xr=(*adresc[ad])()) {res&=~255;res|=255;} else {res&=~255;}
*/
  if(xr=indir()) res|=255; else res&=~255;
}

void leay(void) /* Zx */
{
  ASMOPT(LEAX(xyus+4));
  if(yr=indir()) res|=255; else res&=~255;
/*
  if(yr=(*adresc[ad])()) {res&=~255;res|=255;} else {res&=~255;}
*/
}

void leas(void)
{
  ASMOPT(LEA(xyus+12));
  sr=indir();
/*
  sr=(*adresc[ad])();
*/
}

void leau(void)
{
  ASMOPT(LEA(xyus+8));
  ur=indir();
/*
  ur=(*adresc[ad])();
*/
}

#ifndef __inline__
#define __inline__
#endif

void __inline__ pshsr(int i)
{
  if (i&0x80) {madd_modulo(short,sr,-2);stocl(sr,pc);}
  if (i&0x40) {madd_modulo(short,sr,-2);stocl(sr,ur);}
  if (i&0x20) {madd_modulo(short,sr,-2);stocl(sr,yr);}
  if (i&0x10) {madd_modulo(short,sr,-2);stocl(sr,xr);}
  if (i&0x08) {madd_modulo(short,sr,-1);stocs(sr,dp);}
  if (i&0x04) {madd_modulo(short,sr,-1);stocs(sr,br);}
  if (i&0x02) {madd_modulo(short,sr,-1);stocs(sr,ar);}
  if (i&0x01) {madd_modulo(short,sr,-1);stocs(sr,getcc());}
}

void pshs(void)
{
  pshsr(*op);
}

void __inline__ pulsr(int i)
{
  if (i&0x01) {setcc(loads(sr)); madd_modulo(short, sr, 1);}
  if (i&0x02) {ar=loads(sr); madd_modulo(short, sr, 1);}
  if (i&0x04) {br=loads(sr); madd_modulo(short, sr, 1);}
  if (i&0x08) {dp=loads(sr); madd_modulo(short, sr, 1);}
  if (i&0x10) {xr=loadl(sr); madd_modulo(short, sr, 2);}
  if (i&0x20) {yr=loadl(sr); madd_modulo(short, sr, 2);}
  if (i&0x40) {ur=loadl(sr); madd_modulo(short, sr, 2);}
  if (i&0x80) {pc=loadl(sr); madd_modulo(short, sr, 2);}
}

void puls(void)
{
  pulsr(*op);
}

void pshu(void)
{
  int   i=*op;

  if (i&0x80) {madd_modulo(short,ur,-2);stocl(ur,pc);}
  if (i&0x40) {madd_modulo(short,ur,-2);stocl(ur,sr);}
  if (i&0x20) {madd_modulo(short,ur,-2);stocl(ur,yr);}
  if (i&0x10) {madd_modulo(short,ur,-2);stocl(ur,xr);}
  if (i&0x08) {madd_modulo(short,ur,-1);stocs(ur,dp);}
  if (i&0x04) {madd_modulo(short,ur,-1);stocs(ur,br);}
  if (i&0x02) {madd_modulo(short,ur,-1);stocs(ur,ar);}
  if (i&0x01) {madd_modulo(short,ur,-1);stocs(ur,getcc());}
}

void pulu(void)
{
  int   i=*op;

  if (i&0x01) {setcc(loads(ur)); madd_modulo(short, ur, 1);}
  if (i&0x02) {ar=loads(ur); madd_modulo(short, ur, 1);}
  if (i&0x04) {br=loads(ur); madd_modulo(short, ur, 1);}
  if (i&0x08) {dp=loads(ur); madd_modulo(short, ur, 1);}
  if (i&0x10) {xr=loadl(ur); madd_modulo(short, ur, 2);}
  if (i&0x20) {yr=loadl(ur); madd_modulo(short, ur, 2);}
  if (i&0x40) {sr=loadl(ur); madd_modulo(short, ur, 2);}
  if (i&0x80) {pc=loadl(ur); madd_modulo(short, ur, 2);}
}

void rtsm(void)
{
  ASMOPT("movew _xyus+14,d0; addqw #2,_xyus+14; jbsr _loadl; movew d0,_pc+2")
  pc=loadl(sr);
  madd_modulo(short, sr, 2);
}

void abxm(void)
{
  madd_modulo(short, xr, (br&255));
}

void rtim(void)
{
  pulsr(1);
  if (ccrest&0x80) pulsr(0xfe);
  else pulsr(0x80);
}

void cwai(void)
{
  /* a priori inutile */
  /*    printf("cwai\n");
   */   
}

void mulm(void)         /* ZxCx */
{
  long  k;
  ASMOPT("moveq #0,d0; moveq #0,d1; moveb _ar+3,d0; moveb _br+3,d1; "
	 "muluw d1,d0; tstw d0; sneb _res+3; moveb d0,_br+3; smib _res+2; "
	 "lsrw #8,d0; moveb d0,_ar+3");
  k=(unsigned short)ar*(unsigned short)br;
  ar&=~255;ar|=(k>>8)&255;
  br&=~255;br|=k&255;
  /* C=bit7 de br */
  if(br&128) res=0x101; else res=k?1:0;
}

void swim(void)
{
  ccrest|=0x80;
  pshsr(0xff);
  ccrest|=0x50;
  pc=get_short(buf+0xfffa);
}
