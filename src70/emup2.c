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

  emup2.c : emulateur 6809. 2eme partie

*/

#include "emuto7.h"
#include "monitor.h"

#ifdef AMIGA
#define m1   ((short*)&m1)[1]
#define m2   ((short*)&m2)[1]
#define ovfl ((short*)&ovfl)[1]
#define sign ((short*)&sign)[1]
#define res  ((short*)&res)[1]
#endif

#define NEG(a) \
  "moveq #0,d0;moveb _"#a"+3,d0; moveb d0,_m1+3; "\
  "negw d0; moveb d0,_"#a"+3; moveb d0,_ovfl+3; "\
  "moveb d0,_sign+3; movew d0,_res+2"
void nega(void)         /* H?NxZxVxCx */
{
  ASMOPT(NEG(ar));
  m1=ar; m2=-ar;                /* bit V */
  ar=-ar;
  ovfl=sign=res=ar;
  ar&=255;
}

#define COM(a) \
  "moveb _ovfl+3,_m1+3; "\
  "moveb _"#a"+3,d0; notb d0; moveb d0,_"#a"+3; "\
  "moveb d0,_sign+3; orw #256,d0; movew d0,_res+2"
void coma(void)         /* NxZxV0C1 */
{
  ASMOPT(COM(ar));
  m1=ovfl;
  ar=(~ar)&255;
  sign=ar;
  res=sign|0x100; /* bit C a 1 */
}

#define LSR(a) \
  "moveb _"#a"+3,d0; moveq #0,d1; moveb d1,_sign+3; "\
  "moveb d0,d1; lsrb #1,d0; lslw #8,d1; moveb d0,d1; "\
  "moveb d0,_"#a"+3; movew d1,_res+2"
void lsra(void)         /* N0ZxCx */
{
  ASMOPT(LSR(ar));
  res=(ar&1)<<8;
  ar>>=1;
  sign=0;
  res|=ar;
}

#define ROR(a) \
  "movew _res+2,d0; moveb _"#a"+3,d0; moveb d0,d1; "\
  "lsrw #1,d0; lslw #8,d1; "\
  "moveb d0,_"#a"+3; moveb d0,_sign+3; moveb d0,d1; movew d1,_res+2"
void rora(void)         /* NxZxCx */
{
  int i=ar;
  ASMOPT(ROR(ar));
  ar=(i|(res&0x100))>>1;
  sign=ar;
  res=((i&1)<<8)|sign;
}

#define ASR(a) \
  "moveb _"#a"+3,d0; moveb d0,d1; lslw #8,d1; "\
  "asrb #1,d0; moveb d0,_"#a"+3; moveb d0,d1; "\
  "moveb d0,_sign+3; movew d1,_res+2"
void asra(void)         /* H?NxZxCx */
{
  ASMOPT(ASR(ar));
  res=(ar&1)<<8;
  ar=(ar>>1)|(ar&0x80);
  sign=ar;
  res|=sign;
}

#define ASL(a) \
  "moveq #0,d0; moveb _"#a"+3,d0; moveb d0,_m1+3; moveb d0,_m2+3; "\
  "addw d0,d0; moveb d0,_"#a"+3; "\
  "moveb d0,_sign+3; moveb d0,_ovfl+3; movew d0,_res+2"
void asla(void)         /* H?NxZxVxCx */
{
  int k=ar;
  ASMOPT(ASL(ar));
  m1=m2=k;
  k <<= 1;
  ar = k&255;
  ovfl=sign=res=k;
}

#define ROL(a) \
  "moveb _res+2,d1; andb #1,d1; moveb _"#a"+3,d0; "\
  "moveb d0,_m1+3; moveb d0,_m2+3; addw d0,d0; "\
  "orb d1,d0; moveb d0,_"#a"+3; moveb d0,_sign+3; "\
  "moveb d0,_ovfl+3; movew d0,_res+2"
void rola(void)         /* NxZxVxCx */
{
  int   i;
  ASMOPT(ROL(ar));
  i=ar;
  m1=m2=ar;
  ar=(ar<<1)|((res&0x100)>>8);
  ovfl=sign=res=ar;
  ar&=255;
}

#define DEC(a) \
  "moveb #128,_m2+3; moveb _"#a"+3,d0; moveb d0,_m1+3; "\
  "subqb #1,d0; moveb d0,_"#a"+3; "\
  "moveb d0,_sign+3; moveb d0,_ovfl+3; moveb d0,_res+3"
void deca(void)         /* NxZxVx */
{
  ASMOPT(DEC(ar));
  m1=ar; m2=0x80;
  add_modulo(char,ar,-1);
  ovfl=sign=ar;
  setlsb(res,sign);
}

#define INC(a) \
  "clrb _m2+3; moveb _"#a"+3,d0; moveb d0,_m1+3; "\
  "addqb #1,d0; moveb d0,_"#a"+3; "\
  "moveb d0,_sign+3; moveb d0,_ovfl+3; moveb d0,_res+3"
void inca(void)         /* NxZxVx */
{
  ASMOPT(INC(ar));
  m1=ar; m2=0;
  add_modulo(char,ar,+1);
  ovfl=sign=ar;
  setlsb(res,sign);
}

#define TST(a) \
  "moveb _ovfl+3,_m1+3; moveb _"#a"+3,d0; "\
  "moveb d0,_sign+3; moveb d0,_res+3"
void tsta(void)         /* NxZxV0 */
{
  ASMOPT(TST(ar));
  m1=ovfl;
  sign=ar;
  setlsb(res,sign);
}

#define CLR(a) \
  "moveb _ovfl+3,_m1+3; moveq #0,d0; "\
  "moveb d0,_"#a"+3; moveb d0,_sign+3; movew d0,_res+2"
void clra(void)         /* N0Z1V0C0 */
{
  ASMOPT(CLR(ar));
  ar=0;
  m1=ovfl;
  sign=res=0;
}

void negb(void)         /* H?NxZxVxCx */
{
  ASMOPT(NEG(br));
  m1=br; m2=-br;                /* bit V */
  br=-br;
  ovfl=sign=res=br;
  br&=255;
}

void comb(void)         /* NxZxV0C1 */
{
  ASMOPT(COM(br));
  m1=ovfl;
  br=(~br)&255;
  sign=br;
  res=sign|0x100; /* bit C a 1 */
}

void lsrb(void)         /* N0ZxCx */
{
  ASMOPT(LSR(br));
  res=(br&1)<<8;        /* bit C */
  br>>=1;
  sign=0;
  res|=br;
}

void rorb(void)         /* NxZxCx */
{
  int   i;
  ASMOPT(ROR(br));
  i=br;
  br=(br|(res&0x100))>>1;
  sign=br;
  res=((i&1)<<8)|sign;
}

void asrb(void)         /* H?NxZxCx */
{
  ASMOPT(ASR(br));
  res=(br&1)<<8;
  br=(br>>1)|(br&0x80);
  sign=br;
  res|=sign;
}

void aslb(void)         /* H?NxZxVxCx */
{
  ASMOPT(ASL(br));
  m1=m2=br;
  br<<=1;
  ovfl=sign=res=br;
  br&=255;
}

void rolb(void)         /* NxZxVxCx */
{
  int   i;
  ASMOPT(ROL(br));
  i=br;
  m1=m2=br;
  br=(br<<1)|((res&0x100)>>8);
  ovfl=sign=res=br;
  br&=255;
}

void decb(void)         /* NxZxVx */
{
  ASMOPT(DEC(br));
  m1=br; m2=0x80;
  add_modulo(char,br,-1);
  ovfl=sign=br;
  setlsb(res,sign);
}

void incb(void)         /* NxZxVx */
{
  ASMOPT(INC(br));
  m1=br; m2=0;
  add_modulo(char,br,+1);
  ovfl=sign=br;
  setlsb(res,sign);
}

void tstb(void)         /* NxZxV0 */
{
  ASMOPT(TST(br));
  m1=ovfl;
  sign=br;
  setlsb(res,sign);
}

void clrb(void)         /* N0Z1V0C0 */
{
  ASMOPT(CLR(br));
  br=0;
  m1=ovfl;
  sign=res=0;
}

void suba(void)         /* H?NxZxVxCx */
{
  int   val;
  val=loads((*adresc[ad])());
  val=-val;
  m1=ar; m2=val;
  val+=ar;
  ovfl=sign=res=val;
  ar=val&255;
}

static void cmpab(long *reg, long val)
{
  m1=*reg; val=-val; m2=val;
  ovfl=sign=res=(*reg) + val;
}

void cmpa(void)         /* H?NxZxVxCx */
{
  cmpab(&ar,loads((*adresc[ad])()));
}
void CPAi(void)
{
  cmpab(&ar,255&*op);
}
void CPAe(void)
{
  cmpab(&ar,loads(/*0xFFFF&*/get_short(op)));
}
void CPAx(void)
{
  cmpab(&ar,loads(indir()));
}

void sbca(void)         /* H?NxZxVxCx */
{
  int   val;
  val=loads((*adresc[ad])());
  m1=ar; m2=-val;
  ar-=val+((res&0x100)>>8);
  ovfl=sign=res=ar;
  ar&=255;
}

void subd(void)         /* NxZxVxCx */
{
  long  dr,val;
  val=loadl((*adresl[ad])());
  m1=ar; m2=(-val)>>8;
  dr=(ar<<8)+br-val;
  ar=dr>>8;
  br=dr&255;
  ovfl=sign=res=ar;
  res|=br;
  ar&=255;
}

static void andab(long *reg, long val)
{
  m1=ovfl;
  val&=*reg;
  *reg=val;
  sign=val;
  setlsb(res,sign);
}

void anda(void)         /* NxZxV0 */
{
  andab(&ar,loads((*adresc[ad])()));
}
void ANAi(void)
{
  andab(&ar,255&*op);
}
void ANAe(void)
{
  andab(&ar,loads(get_short(op)/*&0xFFFF*/));
}
void ANAx(void)
{
  andab(&ar,loads(indir()));
}

void bita(void)         /* NxZxV0 */
{
  int   val;
  val=loads((*adresc[ad])());
  m1=ovfl;
  sign=ar&val;
  setlsb(res,sign);
}

static void ldab(long *reg,long v)
{
  m1=ovfl;
  sign=*reg=v;
  setlsb(res,sign);
}

#define VAL_m "lea _adresc,a0; movel _ad,d0; movel a0@(d0:l:4),a0; jbsr a0@; "
#define VAL_e "movew a4@,d0; "
#define VAL_x "jbsr _indir; "
#define VAL_I "moveb a4@,d0; "
#define VAL_M VAL_m##"jbsr _loads; "
#define VAL_E VAL_e##"jbsr _loads; "
#define VAL_X VAL_x##"jbsr _loads; "

#define LD(a,val) \
  "moveb _ovfl+3,_m1+3; "##val \
  "moveb d0,_"#a"+3; moveb d0,_sign+3; moveb d0,_res+3"

void ldam(void)         /* NxZxV0 */
{
  ASMOPT(LD(ar,VAL_M));
  ldab(&ar,loads((*adresc[ad])()));
}
void LDAe(void)
{
  ASMOPT(LD(ar,VAL_E));
  ldab(&ar,loads(get_short(op)&0xffff));
}
void LDAi(void)
{
   ASMOPT(LD(ar,VAL_I));
   ldab(&ar,255&*op);
}
void LDAx(void)
{
   ASMOPT(LD(ar,VAL_X));
   ldab(&ar,loads(indir()));
}
void LDAd(void)
{
   ldab(&ar,loads((dp<<8)|(255&*op)));
}

#define ST(a,val) \
  "moveb _ovfl+3,_m1+3; "##val \
  "moveq #0, d1; moveb _"#a"+3,d1; moveb d1,_sign+3; moveb d1,_res+3; "\
  "jbsr _stocs"
static void stab(long adr, long reg)
{
  m1=ovfl;
  sign=reg;
  setlsb(res,reg);
  stocs(adr,reg);
}
void stam(void)         /* NxZxV0 */
{
  ASMOPT(ST(ar,VAL_m));
  stab((*adresc[ad])(),ar);
}
void STAe(void)
{
  ASMOPT(ST(ar,VAL_e));
  stab(get_short(op)/*&0xFFFF*/,ar);
}
void STAd(void)
{
  stab((dp<<8)|(255&*op),ar);
}
void STAx(void)
{
  ASMOPT(ST(ar,VAL_x));
  stab(indir(),ar);
}

void eora(void)         /* NxZxV0 */
{
  int   val;
  val=loads((*adresc[ad])());
  m1=ovfl;
  val^=ar;
  ar=val;
  sign=val;
  setlsb(res,sign);
}

void adca(void)         /* HxNxZxVxCx */
{
  int   val;
  val=loads((*adresc[ad])());
  m1=h1=ar; m2=val;
  h2=val+((res&0x100)>>8);
  ar+=h2;
  ovfl=sign=res=ar;
  ar&=255;
}

static void orab(long *reg,long val)
{
  m1=ovfl;
  val|=*reg;
  *reg=val;
  sign=val;
  setlsb(res,sign);
}
void oram(void)         /* NxZxV0 */
{
  orab(&ar,loads((*adresc[ad])()));
}
void ORAi(void)
{
  orab(&ar,255&*op);
}
void ORAe(void)
{
  orab(&ar,loads(/*0xFFFF&*/get_short(op)));
}
void ORAx(void)
{
  orab(&ar,loads(indir()));
}

void adda(void)         /* HxNxZxVxCx */
{
  int   val;
  val=loads((*adresc[ad])());
  m1=h1=ar; m2=h2=val;
  ar+=val;
  ovfl=sign=res=ar;
  ar&=255;
}

#define CMPX(a)\
    "moveb _"#a"+2,_m1+3; "\
    "lea _adresl,a0; movel _ad,d0; movel a0@(d0:l:4),a0; jbsr a0@; " \
    "jbsr _loadl; negw d0; movew d0,d1; lsrw #8,d1; moveb d1,_m2+3; "\
    "addw _"#a"+2,d0; smib _res+2; sneb _res+3; smib _sign+3; smib _ovfl+3"
void cmpx(void)         /* NxZxVxCx */
{
  long  val;
  ASMOPT(CMPX(xyus));
  val=-loadl((*adresl[ad])());
  m1=xr>>8; m2=val>>8;val+=xr;
  ovfl=sign=res=val>>8;
  res|=val&255;
}

void bsrm(void)
{
  long k;
  add_modulo(short,sr,-2);
  k=pc;
  add_modulo(short,pc,(char)op[0]);
  stocl(sr,k);
}

#define LDX(a) \
 "moveb _ovfl+3,_m1+3; "\
 "movel _ad,d0; lea _adresl,a0; movel a0@(d0:l:4),a0; jbsr a0@; "\
 "jbsr _loadl; movel d0,_"#a"; tstw d0; sneb _res+3; smib _sign+3"
void ldxm(void) /* NxZxV0 */
{
  ASMOPT(LDX(xyus)); 
  xr=loadl((*adresl[ad])());
  m1=ovfl;
  sign=xr>>8;
  setlsb(res,sign|xr);
}

void LDXi(void)
{
    m1=ovfl;
    xr=(unsigned short)get_short(op);
    sign=xr>>8;
    setlsb(res,xr|sign);
}

#define STX(a) \
 "moveb _ovfl+3,_m1+3; "\
 "movel _ad,d0; lea _adresl,a0; movel a0@(d0:l:4),a0; jbsr a0@; "\
 "movew _"#a"+2,d1; sneb _res+3; smib _sign+3; jbsr _stocl"
void stxm(void)         /* NxZxV0 */
{
  ASMOPT(STX(xyus));
  stocl((*adresl[ad])(),xr);
  m1=0; m2=0x80;
  sign=xr>>8;
  setlsb(res,xr|sign);
}

void subb(void)         /* H?NxZxVxCx */
{
  int   val;
  val=loads((*adresc[ad])());
  val=-val;
  m1=br; m2=val;
  val+=br;
  br=val&255;
  ovfl=sign=res=val;
}

void cmpb(void)         /* H?NxZxVxCx */
{
  cmpab(&br,loads((*adresc[ad])()));
}

void CPBi(void)
{
  cmpab(&br,255&*op);
}

void sbcb(void)         /* H?NxZxVxCx */
{
  int   val;
  val=loads((*adresc[ad])());
  m1=br; m2=-val;
  br-=val+((res&0x100)>>8);
  ovfl=sign=res=br;
  br&=255;
}

void addd(void)         /* NxZxVxCx */
{
  long  dr,val;
  val=loadl((*adresl[ad])());
  m1=ar; m2=val>>8;
  dr=(ar<<8)+br+val;
  ar=dr>>8;
  br=dr&255;
  ovfl=sign=res=ar;
  res|=br;
  ar&=255;
}

void andb(void)         /* NxZxV0 */
{
  andab(&br,loads((*adresc[ad])()));
}

void ANBi(void)
{
  andab(&br,255&*op);
}

void bitb(void)         /* NxZxV0 */
{
  int   val;
  val=loads((*adresc[ad])());
  m1=ovfl;
  sign=br&val;
  setlsb(res,sign);
}

void ldbm(void)         /* NxZxV0 */
{
  ASMOPT(LD(br,VAL_M));
  ldab(&br,loads((*adresc[ad])()));
}
void LDBi(void)
{
  ASMOPT(LD(br,VAL_I));
  ldab(&br,255&*op);
}

void stbm(void)         /* NxZxV0 */
{
  ASMOPT(ST(br,VAL_m));
  stab((*adresc[ad])(),br);
/*
  stocs((*adresc[ad])(),br);
  sign=br;
  m1=ovfl;
  setlsb(res,sign);
*/
}

void eorb(void)         /* NxZxV0 */
{
  int   val;
  val=loads((*adresc[ad])());
  m1=ovfl;
  val^=br;
  br=val;
  sign=br;
  setlsb(res,sign);
}

void adcb(void)         /* HxNxZxVxCx */
{
  int   val;
  val=loads((*adresc[ad])());
  m1=h1=br; m2=val;
  h2=val+((res&0x100)>>8);
  br+=h2;
  ovfl=sign=res=br;
  br&=255;
}

void orbm(void)         /* NxZxV0 */
{
  orab(&br,loads((*adresc[ad])()));
}

void ORBi(void)
{
  orab(&br,255&*op);
}

void addb(void)         /* HxNxZxVxCx */
{
  int   val;
  val=loads((*adresc[ad])());
  m1=h1=br; m2=h2=val;
  br+=val;
  ovfl=sign=res=br;
  br&=255;
}

void lddm(void)         /* NxZxV0 */
{
  long  dr;
  ASMOPT("movel _ad,d0; lea _adresl,a0; movel a0@(d0:l:4),a0; jbsr a0@; "\
         "jbsr _loadl; moveb _ovfl+3,_m1+3; movew d0,d1; lsrw #8,d1; "\
         "moveb d1,_ar+3; moveb d0,_br+3; "\
         "moveb d1,_sign+3; orb d1,d0; moveb d0,_res+3");
  dr=loadl((*adresl[ad])());
  m1=ovfl;
  ar=(dr>>8)&255;
  br=dr&255;
  sign=ar;
  setlsb(res,ar|br);
}
void LDDi(void)
{
    m1=ovfl;
    sign=ar=255&op[0];br=255&op[1];
    setlsb(res,ar|br);
}

void stdm(void)         /* NxZxV0 */
{
  ASMOPT("movel _ad,d0; lea _adresl,a0; movel a0@(d0:l:4),a0; jbsr a0@; "\
         "moveb _ar+3,d1; moveb d1,_sign+3; lslw #8,d1; moveb _br+3,d1; "\
         "tstw d1; sneb _res+3; moveb _ovfl+3,_m1+3; jbsr _stocl");
  stocl((*adresl[ad])(),((sign=ar)<<8)|(br&255));
  m1=ovfl;
/*  sign=ar;*/
  setlsb(res,ar|br);
}

void ldum(void) /* NxZxV0 */
{
  ASMOPT(LDX(xyus+8)); 
  ur=loadl((*adresl[ad])());
  m1=ovfl;
  sign=ur>>8;
  setlsb(res,sign|ur);
}

void LDUi(void)
{
    m1=ovfl;
    ur=(unsigned short)get_short(op);
    sign=ur>>8;
    setlsb(res,ur|sign);
}

void stum(void)         /* NxZxV0 */
{
  ASMOPT(STX(xyus+8)); 
  stocl((*adresl[ad])(),ur);
  m1=ovfl;
  sign=ur>>8;
  setlsb(res,sign|ur);
}
