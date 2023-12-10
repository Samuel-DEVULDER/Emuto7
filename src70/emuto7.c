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

  emuto7.c : emulateur 6809. adressage

  Legere optimisation de l'acces aux registres XYUS par
  Samuel Devulder en 07/97.
*/

#include "emuto7.h"
#include "monitor.h"
#include "video.h"
#ifdef __SASC
#include <string.h> /* seg_/sreg: requiered by SAS C */
#endif

long    pc;
long    xyus[4];
long    ar,br;
long    dp;

long    mask;
int     flto7;
long    endmem;

int     res,m1,m2,sign,ovfl,h1,h2,ccrest;

extern void (*code[])(void);

#if !defined(AMIGA)||!defined(__GNUC__)
char    *op;
#endif
int     ad;
long*   exreg[16];
long*   regist_ind[256];
long    offset_fond;

extern int quitf;

struct code_hlp {
    unsigned short adrs;
    unsigned short size;
    void (*code)(void);
} code_hlp[3*256];

void inito7(char *name)
{
  int   i;
  
  xr=yr=ur=0;
  pc=0xf98d;
  sr=0x8000;
  ar=br=0;
  dp=0;
  res=m1=m2=sign=ovfl=ccrest=0;
  for(i=0;i<16;i++) exreg[i]=NULL;
  exreg[1]=&xr;
  exreg[2]=&yr;
  exreg[3]=&ur;
  exreg[4]=&sr;
  exreg[5]=&pc;
  exreg[8]=&ar;
  exreg[9]=&br;
  exreg[11]=&dp;
  for(i=0;i<256;++i) regist_ind[i]=regist[(i&0x60)>>5];
  mask=0xc0c0;
  iniscreen(name);
  pa1=pb1=0;
  for(i=0;i<3*256;++i) {
      code_hlp[i].adrs = adr[i];
      code_hlp[i].size = taille[i];
      code_hlp[i].code = code[i];
  }
}

long getcc(void)
{
  return        ((((h1&15)+(h2&15))&16)<<1)
    |((sign&0x80)>>4)
      |((((res&0xff)==0)&1)<<2)
        |(( ((~(m1^m2))&(m1^ovfl)) &0x80)>>6)
          |((res&0x100)>>8)
            |ccrest;
}

void setcc(int i)
{
  m1=m2=0;
  res=((i&1)<<8)|(4-(i&4));
  ovfl=(i&2)<<6;
  sign=(i&8)<<4;
  h1=h2=(i&32)>>2;
  ccrest=i&0xd0;
}

long direc(void)
{
  ASMOPT("moveq #0,d0; movew _dp+3,d0; moveb "OP_REG"@,d0");
  return (dp<<8)+((*op)&255);
}

long inher(void)
{
  return -1;
}

long immedc(void)
{
  long r;
  ASMOPT("movel _pc,d0; subqw #1,d0");
  r=pc;
  add_modulo(short,r,-1);
  return r;
}

long immedl(void)
{
  long r;
  ASMOPT("movel _pc,d0; subqw #2,d0");
  r=pc;
  add_modulo(short,r,-2);
  return r;
}

long indxp(long _op)
{
  long  *x=regist_ind[_op];
  long  k=*x;
  madd_modulo(short,*x,1);
  return k;
}

long indxpp(long _op)
{
  long  *x=regist_ind[_op];
  long  k=*x;
  madd_modulo(short,*x,2);
  return k;
}

long indmx(long _op)
{
  long  *x=regist_ind[_op];
  madd_modulo(short,*x,-1);
  return *x;
}

long indmmx(long _op)
{
  long  *x=regist_ind[_op];
  madd_modulo(short,*x,-2);
  return *x;
}

long indx(long _op)
{
  return *regist_ind[_op];
}

long indax(long _op)
{
  char  a=ar;
  long  k=*regist_ind[_op];
  add_modulo(short,k,a);
  return k;
}

long indbx(long _op)
{
  char  b=br;
  long  k=*regist_ind[_op];
  add_modulo(short,k,b);
  return k;
}

long inder(void)
{
  return 0;
}       

long ind1x(long _op)
{
  char  del=op[1];
  long  k=*regist_ind[_op];
  madd_modulo(short,pc,1);
  add_modulo(short,k,del);
  return k;
}

long ind2x(long _op)
{
  short del=get_short(op+1);
  long  k=*regist_ind[_op];
  madd_modulo(short,pc,2);
  add_modulo(short,k,del);
  return k;
}

long inddx(long _op)
{
  short del=((ar<<8)&~255)|(br&255);
  long  k=*regist_ind[_op];
  add_modulo(short,k,del);
  return k;
}

long ind1p(void)
{
  char  del=op[1];
  long  k;
  madd_modulo(short,pc,1); k=pc;
  add_modulo(short,k,del);
  return k;
}

long ind2p(void)
{
  short del=get_short(op+1);
  long  k;
  madd_modulo(short,pc,2); k = pc;
  add_modulo(short,k,del);
  return k;
}

long indad(void)
{
  madd_modulo(short,pc,2);
  return get_short(op+1);
}

#define IND_I(x,reg) \
 case (0x00 + x): return  0x00 + reg; break;\
 case (0x01 + x): return  0x01 + reg; break;\
 case (0x02 + x): return  0x02 + reg; break;\
 case (0x03 + x): return  0x03 + reg; break;\
 case (0x04 + x): return  0x04 + reg; break;\
 case (0x05 + x): return  0x05 + reg; break;\
 case (0x06 + x): return  0x06 + reg; break;\
 case (0x07 + x): return  0x07 + reg; break;\
 case (0x08 + x): return  0x08 + reg; break;\
 case (0x09 + x): return  0x09 + reg; break;\
 case (0x0a + x): return  0x0a + reg; break;\
 case (0x0b + x): return  0x0b + reg; break;\
 case (0x0c + x): return  0x0c + reg; break;\
 case (0x0d + x): return  0x0d + reg; break;\
 case (0x0e + x): return  0x0e + reg; break;\
 case (0x0f + x): return  0x0f + reg; break;\
 case (0x10 + x): return -0x10 + reg; break;\
 case (0x11 + x): return -0x0f + reg; break;\
 case (0x12 + x): return -0x0e + reg; break;\
 case (0x13 + x): return -0x0d + reg; break;\
 case (0x14 + x): return -0x0c + reg; break;\
 case (0x15 + x): return -0x0b + reg; break;\
 case (0x16 + x): return -0x0a + reg; break;\
 case (0x17 + x): return -0x09 + reg; break;\
 case (0x18 + x): return -0x08 + reg; break;\
 case (0x19 + x): return -0x07 + reg; break;\
 case (0x1a + x): return -0x06 + reg; break;\
 case (0x1b + x): return -0x05 + reg; break;\
 case (0x1c + x): return -0x04 + reg; break;\
 case (0x1d + x): return -0x03 + reg; break;\
 case (0x1e + x): return -0x02 + reg; break;\
 case (0x1f + x): return -0x01 + reg; break;

#define IND_X(x,op) \
 case (0x00 + x): return indxp(op);  break; \
 case (0x01 + x): return indxpp(op); break; \
 case (0x02 + x): return indmx(op);  break; \
 case (0x03 + x): return indmmx(op); break; \
 case (0x04 + x): return indx(op);   break; \
 case (0x05 + x): return indbx(op);  break; \
 case (0x06 + x): return indax(op);  break; \
 case (0x07 + x): return inder();    break; \
 case (0x08 + x): return ind1x(op);  break; \
 case (0x09 + x): return ind2x(op);  break; \
 case (0x0a + x): return inder();    break; \
 case (0x0b + x): return inddx(op);  break; \
 case (0x0c + x): return ind1p();    break; \
 case (0x0d + x): return ind2p();    break; \
 case (0x0e + x): return inder();    break; \
 case (0x0f + x): return indad();    break; \
 case (0x10 + x): return 0; /*get_short(buf+indxp(op));*/  break; \
 case (0x11 + x): return get_short(buf+indxpp(op)); break; \
 case (0x12 + x): return 0; /*get_short(buf+indmx(op));*/  break; \
 case (0x13 + x): return get_short(buf+indmmx(op)); break; \
 case (0x14 + x): return get_short(buf+indx(op));   break; \
 case (0x15 + x): return get_short(buf+indbx(op));  break; \
 case (0x16 + x): return get_short(buf+indax(op));  break; \
 case (0x17 + x): return 0;                         break; \
 case (0x18 + x): return get_short(buf+ind1x(op));  break; \
 case (0x19 + x): return get_short(buf+ind2x(op));  break; \
 case (0x1a + x): return 0;                         break; \
 case (0x1b + x): return get_short(buf+inddx(op));  break; \
 case (0x1c + x): return get_short(buf+ind1p());    break; \
 case (0x1d + x): return get_short(buf+ind2p());    break; \
 case (0x1e + x): return 0;                         break; \
 case (0x1f + x): return get_short(buf+indad());    break;

#if 1
long indir(void)
{
  switch((unsigned char)*op) {
      IND_I(0x00,xr)
      IND_I(0x20,yr)
      IND_I(0x40,ur)
      IND_I(0x60,sr)
      IND_X(0x80,(unsigned char)*op)
      IND_X(0xa0,(unsigned char)*op)
      IND_X(0xc0,(unsigned char)*op)
      IND_X(0xe0,(unsigned char)*op)
      default: return 0; break;
  }
}
#else
long indir(void)
{
  unsigned char _op=0;
  unsigned char *BUF;

  _op=*op;
  if((_op&128)==0) {
    unsigned int k=_op&31; if(k&16) k|=~31;
    return (*regist_ind[_op])+k; /* (*regist[_op>>5])+k; */
  } else {
    BUF=buf;
    switch((unsigned char)_op) {
        IND_X(0x00)
	IND_X(0x20)
	IND_X(0x40)
	IND_X(0x60)
	IND_X(0x80)
	IND_X(0xa0)
	IND_X(0xc0)
	IND_X(0xe0)
    }
    return 0;
  }
/*
  long  k;
  unsigned int _op = (unsigned char)*op;
  
  if (_op&0x80)
    {
      k=(*indmod[_op&15])();
      if (_op&0x10) {unsigned char *b=buf+k;return get_short(b);}
      return k;
    }
  if (_op&0x10)
    {
      if (_op&15) return (*(regist[(_op&0x60)>>5]))-((-_op)&15);
      return (*(regist[(_op&0x60)>>5]))-16;
    }
  return (*(regist[(_op&0x60)>>5]))+(_op&15);
  */
}
#endif

long etend(void)
{
  return get_short(op);
}

long (*adresc[])(void)=
{direc,inher,inher,immedc,indir,etend,inher};

long (*adresl[])(void)=
{direc,inher,inher,immedl,indir,etend,inher};

void what(void)
{
}

void cd10(void); void cd11(void);
void JSRe(void); void JSRx(void);

void (*code[])(void)=
/*00   01   02   03   04   05   06   07   08   09   0A   0B   0C   0D   0E   0F */
{negm,what,iosb,comm,lsrm,what,rorm,asrm,aslm,rolm,decm,what,incm,tstm,jmpm,clrm /* 00 */
,cd10,cd11,nopm,synm,what,what,lbra,lbsr,what,daam,orcc,what,andc,sexm,exgm,tfrm /* 10 */
,bras,brns,bhis,blss,bccs,blos,bnes,beqs,bvcs,bvss,bpls,bmis,bges,blts,bgts,bles /* 20 */
,leax,leay,leas,leau,pshs,puls,pshu,pulu,what,rtsm,abxm,rtim,cwai,mulm,what,swim /* 30 */
,nega,what,what,coma,lsra,what,rora,asra,asla,rola,deca,what,inca,tsta,what,clra /* 40 */
,negb,what,what,comb,lsrb,what,rorb,asrb,aslb,rolb,decb,what,incb,tstb,what,clrb /* 50 */
,negm,what,what,comm,lsrm,what,rorm,asrm,aslm,rolm,decm,what,incm,tstm,jmpm,clrm /* 60 */
,negm,what,what,comm,lsrm,what,rorm,asrm,aslm,rolm,decm,what,incm,tstm,jmpm,clrm /* 70 */
,suba,CPAi,sbca,subd,ANAi,bita,LDAi,what,eora,adca,ORAi,adda,cmpx,bsrm,LDXi,what /* 80 */
,suba,cmpa,sbca,subd,anda,bita,LDAd,STAd,eora,adca,oram,adda,cmpx,jsrm,ldxm,stxm /* 90 */
,suba,CPAx,sbca,subd,ANAx,bita,LDAx,STAx,eora,adca,ORAx,adda,cmpx,JSRx,ldxm,stxm /* A0 */
,suba,CPAe,sbca,subd,ANAe,bita,LDAe,STAe,eora,adca,ORAe,adda,cmpx,JSRe,ldxm,stxm /* B0 */
,subb,CPBi,sbcb,addd,ANBi,bitb,LDBi,what,eorb,adcb,ORBi,addb,LDDi,what,LDUi,what /* C0 */
,subb,cmpb,sbcb,addd,andb,bitb,ldbm,stbm,eorb,adcb,orbm,addb,lddm,stdm,ldum,stum /* D0 */
,subb,cmpb,sbcb,addd,andb,bitb,ldbm,stbm,eorb,adcb,orbm,addb,lddm,stdm,ldum,stum /* E0 */
,subb,cmpb,sbcb,addd,andb,bitb,ldbm,stbm,eorb,adcb,orbm,addb,lddm,stdm,ldum,stum /* F0 */

,what,what,what,what,what,what,what,what,what,what,what,what,what,what,what,what
,what,what,what,what,what,what,what,what,what,what,what,what,what,what,what,what
,what,lbrn,lbhi,lbls,lbcc,lblo,lbne,lbeq,lbvc,lbvs,lbpl,lbmi,lbge,lblt,lbgt,lble
,what,what,what,what,what,what,what,what,what,what,what,what,what,what,what,swi2
,what,what,what,what,what,what,what,what,what,what,what,what,what,what,what,what
,what,what,what,what,what,what,what,what,what,what,what,what,what,what,what,what
,what,what,what,what,what,what,what,what,what,what,what,what,what,what,what,what
,what,what,what,what,what,what,what,what,what,what,what,what,what,what,what,what
,what,what,what,cmpd,what,what,what,what,what,what,what,what,cmpy,what,ldym,what
,what,what,what,cmpd,what,what,what,what,what,what,what,what,cmpy,what,ldym,stym
,what,what,what,cmpd,what,what,what,what,what,what,what,what,cmpy,what,ldym,stym
,what,what,what,cmpd,what,what,what,what,what,what,what,what,cmpy,what,ldym,stym
,what,what,what,what,what,what,what,what,what,what,what,what,what,what,ldsm,what
,what,what,what,what,what,what,what,what,what,what,what,what,what,what,ldsm,stsm
,what,what,what,what,what,what,what,what,what,what,what,what,what,what,ldsm,stsm
,what,what,what,what,what,what,what,what,what,what,what,what,what,what,ldsm,stsm

,what,what,what,what,what,what,what,what,what,what,what,what,what,what,what,what
,what,what,what,what,what,what,what,what,what,what,what,what,what,what,what,what
,what,what,what,what,what,what,what,what,what,what,what,what,what,what,what,what
,what,what,what,what,what,what,what,what,what,what,what,what,what,what,what,swi3
,what,what,what,what,what,what,what,what,what,what,what,what,what,what,what,what
,what,what,what,what,what,what,what,what,what,what,what,what,what,what,what,what
,what,what,what,what,what,what,what,what,what,what,what,what,what,what,what,what
,what,what,what,what,what,what,what,what,what,what,what,what,what,what,what,what
,what,what,what,cmpu,what,what,what,what,what,what,what,what,cmps,what,what,what
,what,what,what,cmpu,what,what,what,what,what,what,what,what,cmps,what,what,what
,what,what,what,cmpu,what,what,what,what,what,what,what,what,cmps,what,what,what
,what,what,what,cmpu,what,what,what,what,what,what,what,what,cmps,what,what,what
,what,what,what,what,what,what,what,what,what,what,what,what,what,what,what,what
,what,what,what,what,what,what,what,what,what,what,what,what,what,what,what,what
,what,what,what,what,what,what,what,what,what,what,what,what,what,what,what,what
,what,what,what,cmpu,what,what,what,what,what,what,what,what,cmps,what,what,what
};

/*
#ifdef AMIGA
#define ad ((char*)&ad)[3]
#endif
*/

void cd10(void)
{
  struct code_hlp *ch;
  char *op_;
#ifdef OP_REG
  ASMOPT("
        clrl d0
        moveb "OP_REG"@+,d0
        lea _code_hlp+8*256,a0
        lea a0@(d0:l:8),a0
        movew a0@+,_ad+2
        addw a0@+,d1
        movew d1,_pc+2
        movel a0@,a0
        jbsr a0@")
#endif
/*
#ifdef OP_REG
  ASMOPT("
        movel _pc,d0
        movel d0,d1
        movel d0,"OP_REG"
        addl #_buf+1,"OP_REG"
        andw #-8192,d0
        cmpw #16384,d0
        jne L_1
        addl _offset_fond,"OP_REG"
L_1:    clrl d0
        moveb "OP_REG"@+,d0
        lea _code_hlp+8*256,a0
        lea a0@(d0:l:8),a0
        movew a0@+,_ad+2
        addw a0@+,d1
        movew d1,_pc+2
        movel a0@,a0
        jbsr a0@")
#endif
*/
/*
  op_= &buf[pc+1];
  if (short_msk(pc,0xE000,0x4000)) op_ += offset_fond;
*/
  ch = &(code_hlp+256)[(unsigned char)(*op_++)];
  op = op_;
  ad = ch->adrs;
  madd_modulo(short, pc, ch->size);
  (*(ch->code))();
}

void cd11(void)
{
  struct code_hlp *ch;
  char *op_;
#ifdef OP_REG
  ASMOPT("
        clrl d0
        moveb "OP_REG"@+,d0
        lea _code_hlp+8*512,a0
        lea a0@(d0:l:8),a0
        movew a0@+,_ad+2
        addw a0@+,d1
        movew d1,_pc+2
        movel a0@,a0
        jbsr a0@")
#endif
/*
#ifdef OP_REG
  ASMOPT("
        movel _pc,d0
        movel d0,d1
        movel d0,"OP_REG"
        addl #_buf+1,"OP_REG"
        andw #-8192,d0
        cmpw #16384,d0
        jne L_2
        addl _offset_fond,"OP_REG"
L_2:    clrl d0
        moveb "OP_REG"@+,d0
        lea _code_hlp+8*512,a0
        lea a0@(d0:l:8),a0
        movew a0@+,_ad+2
        addw a0@+,d1
        movew d1,_pc+2
        movel a0@,a0
        jbsr a0@")
#endif
*/
/*
  op_= &buf[pc+1];
  if (short_msk(pc,0xE000,0x4000)) op_ += offset_fond;
*/
  ch = &(code_hlp+512)[(unsigned char)(*op_++)];
  op = op_;
  ad = ch->adrs;
  madd_modulo(short, pc, ch->size);
  (*(ch->code))();
}

void exe6809(void)
{
  struct code_hlp *ch;
  char *op_;
#ifdef OP_REG
  ASMOPT("
        movel _pc,d0
        movel d0,d1
        movel d0,"OP_REG"
        addl #_buf,"OP_REG"
        andw #-8192,d0
        cmpw #16384,d0
        jne L_3
        addl _offset_fond,"OP_REG"
L_3:    clrl d0
        moveb "OP_REG"@+,d0
        lea _code_hlp,a0
        lea a0@(d0:l:8),a0
        movew a0@+,_ad+2
        addw a0@+,d1
        movew d1,_pc+2
        movel a0@,a0
        jbsr a0@")
#endif
  op_ = &buf[pc];
  if (short_msk(pc,0xE000,0x4000)) op_ += offset_fond;
  ch = &code_hlp[(unsigned char)(*op_++)];
  op = op_;
  ad = ch->adrs;
  madd_modulo(short, pc, ch->size);
  (*(ch->code))();
}

#ifdef ad
#undef ad
#endif

void printreg(void)
{
  printf(" PC    A   B    X     Y     U     S   DP  CC\n");
  printhx(pc,4); printf("  ");
  printhx(ar,2); printf("  ");
  printhx(br,2); printf("  ");
  printhx(xr,4); printf("  ");
  printhx(yr,4); printf("  ");
  printhx(ur,4); printf("  ");
  printhx(sr,4); printf("  ");
  printhx(dp,2); printf("  ");
  printhx(getcc(),2); printf("  ");
  printf("\n");
}

void printind(void)
{
  printf(" m1    m2    res  sign  ovfl   h1    h2\n");
  printhx(m1,4);   printf("  ");
  printhx(m2,4);   printf("  ");
  printhx(res,4);  printf("  ");
  printhx(sign,4); printf("  ");
  printhx(ovfl,4); printf("  ");
  printhx(h1,4);   printf("  ");
  printhx(h2,4);   printf("  ");
  printf("\n");
}

void setreg(char *c,long k)
{
  if (!strcmp(c,"pc")) pc=k&0xffff;
  if (!strcmp(c,"a"))  ar=k&0xff;
  if (!strcmp(c,"b"))  br=k&0xff;
  if (!strcmp(c,"x"))  xr=k&0xffff;
  if (!strcmp(c,"y"))  yr=k&0xffff;
  if (!strcmp(c,"u"))  ur=k&0xffff;
  if (!strcmp(c,"s"))  sr=k&0xffff;
  if (!strcmp(c,"dp")) dp=k&0xff;
  if (!strcmp(c,"cc")) setcc(k);
}
