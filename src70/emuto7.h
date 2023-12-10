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

  emuto7.h : header partie emulateur

  Legere optimisation de l'acces aux registres XYUS par
  Samuel Devulder en 07/97.
*/

#if defined(__mc68000)||defined(__SASC) /* optimisation de sam */
#define get_short(b) (*(unsigned short*)(b))
#else
#define get_short(b) ((((b)[0]&255)<<8)|((b)[1]&255))
#endif

#define MSKT(TYPE) ((1<<(8*sizeof(TYPE)))-1)
#ifdef __SASC
#define add_modulo(TYPE,V,I)  {unsigned TYPE t;t=V;t+=I;V=t;}
#else
#define add_modulo(TYPE,V,I) {TYPE t;t=V;t+=I;V&=~MSKT(TYPE);V|=((int)t)&MSKT(TYPE);}
#endif
/* {int t;t=V;t+=I;V&=~MSKT(TYPE);V|=t&MSKT(TYPE);} */
/* {unsigned TYPE t;t=V;t+=I;V=t;} */
/* sam: ou bien ceci V=(V+I)&((1<<(8*sizeof(TYPE)))-1) */
#if defined(__mc68000)||defined(__SASC)
#define madd_modulo(TYPE,V,I) \
{if(sizeof(TYPE)==2) ((short*)&V)[1]+=I; else ((char *)&V)[3]+=I; }
#else
#define madd_modulo(TYPE,V,I) add_modulo(TYPE,V,I)
#endif

#if defined(__GNUC__)&&defined(AMIGA)
#define REGPARM __attribute__((regparm(4)))
#define ASMOPT(code) {__asm__ __volatile__(code: : : "cc");return;}
#else
#define REGPARM
#define ASMOPT(code)
#endif

#define short_msk(val,m1,m2) (((short)(val))&((short)(m1)))==(short)(m2)
#define setlsb(a_,b_) {a_&=~255;a_|=(b_)&255;}

extern  long    pc;
extern  long    xyus[4]; /* sam: was long */
#define xr      xyus[0]      /* optimisation de sam: une indirection de moins */
#define yr      xyus[1]
#define ur      xyus[2]
#define sr      xyus[3]
extern  long    ar,br;
extern  long    dp;

extern int res,m1,m2,sign,ovfl,h1,h2,ccrest;

#if defined(AMIGA)&&defined(__GNUC__)
#define OP_REG "a4"
register char *op asm (OP_REG);
#else
extern  char    *op;
#endif
extern  int      ad;
#define regist &xyus /* optimisation de sam */
extern  long*   (exreg[16]);
extern  long*   regist_ind[256];

extern  int     crayx;
extern  int     crayy;
extern  long    offset_fond;

long getcc(void);
void setcc(int i);

extern long (*adresc[])(void);
extern long (*adresl[])(void);

long loads(long k) REGPARM;
long loadl(long k) REGPARM;
int stocs(long k,long val) REGPARM;
int stocl(long k,long val) REGPARM;

long indir(void);
long direc(void);
long immedc(void);
long immedl(void);

void pshsr(int i);
void pulsr(int i);

void iosb(void);
/*
#define LDAi ldam
#define LDBi ldbm
#define CPAi cmpa
#define CPBi cmpb
#define LDXi ldxm
#define LDAe ldam
#define STAe stam
#define LDDi lddm
#define LDUi ldum
*/
void LDAd(void);
void LDAe(void);
void LDAi(void);
void LDAx(void);
void LDBi(void);
void CPAi(void);
void CPAe(void);
void CPAx(void);
void CPBi(void);
void LDXi(void);
void LDUi(void);
void LDDi(void);
void ORAi(void);
void ORAe(void);
void ORAx(void);
void ORBi(void);
void ANAi(void);
void ANAe(void);
void ANAx(void);
void ANBi(void);
void STAd(void);
void STAe(void);
void STAx(void);

void negm(void);
void comm(void);
void lsrm(void);
void rorm(void);
void asrm(void);
void aslm(void);
void rolm(void);
void decm(void);
void incm(void);
void tstm(void);
void jmpm(void);
void clrm(void);

void nopm(void);
void synm(void);
void lbra(void);
void lbsr(void);
void daam(void);
void orcc(void);
void andc(void);
void sexm(void);
void exgm(void);
void tfrm(void);

void bras(void);
void brns(void);
void bhis(void);
void blss(void);
void bccs(void);
void blos(void);
void bnes(void);
void beqs(void);
void bvcs(void);
void bvss(void);
void bpls(void);
void bmis(void);
void bges(void);
void blts(void);
void bgts(void);
void bles(void);

void leax(void);
void leay(void);
void leas(void);
void leau(void);
void pshs(void);
void puls(void);
void pshu(void);
void pulu(void);
void rtsm(void);
void abxm(void);
void rtim(void);
void cwai(void);
void mulm(void);
void swim(void);

void nega(void);
void coma(void);
void lsra(void);
void rora(void);
void asra(void);
void asla(void);
void rola(void);
void deca(void);
void inca(void);
void tsta(void);
void clra(void);

void negb(void);
void comb(void);
void lsrb(void);
void rorb(void);
void asrb(void);
void aslb(void);
void rolb(void);
void decb(void);
void incb(void);
void tstb(void);
void clrb(void);

void suba(void);
void cmpa(void);
void sbca(void);
void subd(void);
void anda(void);
void bita(void);
void ldam(void);
void stam(void);
void eora(void);
void adca(void);
void oram(void);
void adda(void);
void cmpx(void);
void bsrm(void);
void ldxm(void);
void stxm(void);
void jsrm(void);

void subb(void);
void cmpb(void);
void sbcb(void);
void addd(void);
void andb(void);
void bitb(void);
void ldbm(void);
void stbm(void);
void eorb(void);
void adcb(void);
void orbm(void);
void addb(void);
void lddm(void);
void stdm(void);
void ldum(void);
void stum(void);

void lbrn(void);
void lbhi(void);
void lbls(void);
void lbcc(void);
void lblo(void);
void lbne(void);
void lbeq(void);
void lbvc(void);
void lbvs(void);
void lbpl(void);
void lbmi(void);
void lbge(void);
void lblt(void);
void lbgt(void);
void lble(void);

void swi2(void);
void cmpd(void);
void cmpy(void);
void ldym(void);
void stym(void);
void ldsm(void);
void stsm(void);
void swi3(void);
void cmpu(void);
void cmps(void);
