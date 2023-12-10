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

  emup3.c : emulateur 6809. 3eme partie

*/

#include "emuto7.h"
#include "monitor.h"

#ifdef AMIGA
#define m1   ((short*)&m1)[1]
#define m2   ((short*)&m2)[1]
#define ovfl ((short*)&ovfl)[1]
#define sign ((short*)&sign)[1]
#endif

static void local_lbra() /* sam */
{
  madd_modulo(short, pc, get_short(op));
}

void lbrn()
{
}

void lbhi()	/* c|z=0 */
{
  if ((!(res&0x100))&&(res&0xff)) local_lbra();
}
void lbls()	/* c|z=1 */
{
  if ((res&0x100)||(!(res&0xff))) local_lbra();
}

void lbcc()	/* c=0 */
{
  if (!(res&0x100)) local_lbra();
}
void lblo()	/* c=1 */
{
  if (res&0x100) local_lbra();
}

void lbne()	/* z=0 */
{
  if (res&0xff) local_lbra();
}
void lbeq()	/* z=1 */
{
  if (!(res&0xff)) local_lbra();
}

void lbvc()	/* v=0 */
{
  if ( ((m1^m2)&0x80)||(!((m1^ovfl)&0x80)) )
    local_lbra();
}
void lbvs()	/* v=1 */
{
  if ( (!((m1^m2)&0x80))&&((m1^ovfl)&0x80) )
    local_lbra();
}

void lbpl()	/* n=0 */
{
  if (!(sign&0x80)) local_lbra();
}
void lbmi()	/* n=1 */
{
  if (sign&0x80) local_lbra();
}

void lbge()	/* n^v=0 */
{
  if (!((sign^((~(m1^m2))&(m1^ovfl)))&0x80))
    local_lbra();
}
void lblt()	/* n^v=1 */
{
  if ((sign^((~(m1^m2))&(m1^ovfl)))&0x80)
    local_lbra();
}

void lbgt()	/* z|(n^v)=0 */
{
  if ( (res&0xff)
      &&(!((sign^((~(m1^m2))&(m1^ovfl)))&0x80)) )
    local_lbra();
}
void lble()	/* z|(n^v)=1 */
{
  if ( (!(res&0xff))
      ||((sign^((~(m1^m2))&(m1^ovfl)))&0x80) )
    local_lbra();
}

void swi2()
{
  ccrest|=0x80;
  pshsr(0xff);
  pc=get_short(buf+0xfff4);
}

void swi3()
{
  ccrest|=0x80;
  pshsr(0xff);
  pc=get_short(buf+0xfff2);
}

void cmpd()		/* NxZxVxCx */
{
  long	dr,val;
  val=-loadl((*adresl[ad])());
  m1=ar; m2=val>>8;
  dr=(ar<<8)+br+val;
  ovfl=sign=res=dr>>8;
  res|=(dr&255);
}

void cmpy()		/* NxZxVxCx */
{
  long	val;
  val=-loadl((*adresl[ad])());
  m1=yr>>8; m2=val>>8;val+=yr;
  ovfl=sign=res=val>>8;
  res|=val&255;
}

void cmpu()		/* NxZxVxCx */
{
  long	val;
  val=-loadl((*adresl[ad])());
  m1=ur>>8; m2=val>>8; val+=ur;
  ovfl=sign=res=val>>8;
  res|=val&255;
}

void cmps()		/* NxZxVxCx */
{
  long	val;
  val=-loadl((*adresl[ad])());
  m1=sr>>8; m2=val>>8;val+=sr;
  ovfl=sign=res=val>>8;
  res|=val&255;
}

void ldym()	/* NxZxV0 */
{
  yr=loadl((*adresl[ad])());
  m1=ovfl;
  sign=yr>>8;
  setlsb(res,sign|yr);
}

void stym()		/* NxZxV0 */
{
  stocl((*adresl[ad])(),yr);
  m1=ovfl;
  sign=yr>>8;
  setlsb(res,sign|yr);
}

void ldsm()	/* NxZxV0 */
{
  sr=loadl((*adresl[ad])());
  m1=ovfl;
  sign=sr>>8;
  setlsb(res,sign|sr);
}

void stsm()		/* NxZxV0 */
{
  stocl((*adresl[ad])(),sr);
  m1=ovfl;
  sign=sr>>8;
  setlsb(res,sign|sr);
}
