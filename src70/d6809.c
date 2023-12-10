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

 d6809.c : desassembleur 6809

*/

#include "monitor.h"

int taille[]=
{2,2,1,2,2,1,2,2,2,2,2,1,2,2,2,2
,0,0,1,1,1,1,3,3,1,1,2,1,2,1,2,2
,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
,2,2,2,2,2,2,2,2,1,1,1,1,2,1,1,1
,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
,2,1,1,2,2,1,2,2,2,2,2,1,2,2,2,2
,3,1,1,3,3,1,3,3,3,3,3,1,3,3,3,3
,2,2,2,3,2,2,2,1,2,2,2,2,3,2,3,1
,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3
,2,2,2,3,2,2,2,1,2,2,2,2,3,1,3,1
,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3

,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
,1,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4
,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2
,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
,1,1,1,4,1,1,1,1,1,1,1,1,4,1,4,1
,1,1,1,3,1,1,1,1,1,1,1,1,3,1,3,3
,1,1,1,3,1,1,1,1,1,1,1,1,3,1,3,3
,1,1,1,4,1,1,1,1,1,1,1,1,4,1,4,4
,1,1,1,1,1,1,1,1,1,1,1,1,1,1,4,1
,1,1,1,1,1,1,1,1,1,1,1,1,1,1,3,3
,1,1,1,1,1,1,1,1,1,1,1,1,1,1,3,3
,1,1,1,1,1,1,1,1,1,1,1,1,1,1,4,4

,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
,1,1,1,4,1,1,1,1,1,1,1,1,4,1,1,1
,1,1,1,3,1,1,1,1,1,1,1,1,3,1,1,1
,1,1,1,3,1,1,1,1,1,1,1,1,3,1,1,1
,1,1,1,4,1,1,1,1,1,1,1,1,4,1,1,1
,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
};

char mne[]="\
NEG ??  ??  COM LSR ??  ROR ASR ASL ROL DEC ??  INC TST JMP CLR \
--  --  NOP SYNC??  ??  LBRALBSR??  DAA ORCC??  ANDCSEX EXG TFR \
BRA BRN BHI BLS BCC BLO BNE BEQ BVC BVS BPL BMI BGE BLT BGT BLE \
LEAXLEAYLEASLEAUPSHSPULSPSHUPULU??  RTS ABX RTI CWAIMUL ??  SWI \
NEGA??  ??  COMALSRA??  RORAASRAASLAROLADECA??  INCATSTA??  CLRA\
NEGB??  ??  COMBLSRB??  RORBASRBLSLBROLBDECB??  INCBTSTB??  CLRB\
NEG ??  ??  COM LSR ??  ROR ASR ASL ROL DEC ??  INC TST JMP CLR \
NEG ??  ??  COM LSR ??  ROR ASR ASL ROL DEC ??  INC TST JMP CLR \
SUBACMPASBCASUBDANDABITALDA ??  EORAADCAORA ADDACMPXBSR LDX ??  \
SUBACMPASBCASUBDANDABITALDA STA EORAADCAORA ADDACMPXJSR LDX STX \
SUBACMPASBCASUBDANDABITALDA STA EORAADCAORA ADDACMPXJSR LDX STX \
SUBACMPASBCASUBDANDABITALDA STA EORAADCAORA ADDACMPXJSR LDX STX \
SUBBCMPBSBCBADDDANDBBITBLDB ??  EORBADCBORB ADDBLDD ??  LDU ??  \
SUBBCMPBSBCBADDDANDBBITBLDB STB EORBADCBORB ADDBLDD STD LDU STU \
SUBBCMPBSBCBADDDANDBBITBLDB STB EORBADCBORB ADDBLDD STD LDU STU \
SUBBCMPBSBCBADDDANDBBITBLDB STB EORBADCBORB ADDBLDD STD LDU STU \
??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  \
??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  \
??  LBRNLBHILBLSLBCCLBLOLBNELBEQLBVCLBVSLBPLLBMILBGELBLTLBGTLBLE\
??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  SWI2\
??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  \
??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  \
??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  \
??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  \
??  ??  ??  CMPD??  ??  ??  ??  ??  ??  ??  ??  CMPY??  LDY ??  \
??  ??  ??  CMPD??  ??  ??  ??  ??  ??  ??  ??  CMPY??  LDY STY \
??  ??  ??  CMPD??  ??  ??  ??  ??  ??  ??  ??  CMPY??  LDY STY \
??  ??  ??  CMPD??  ??  ??  ??  ??  ??  ??  ??  CMPY??  LDY STY \
??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  LDS ??  \
??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  LDS STS \
??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  LDS STS \
??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  LDS STS \
??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  \
??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  \
??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  \
??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  SWI3\
??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  \
??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  \
??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  \
??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  \
??  ??  ??  CMPU??  ??  ??  ??  ??  ??  ??  ??  CMPS??  ??  ??  \
??  ??  ??  CMPU??  ??  ??  ??  ??  ??  ??  ??  CMPS??  ??  ??  \
??  ??  ??  CMPU??  ??  ??  ??  ??  ??  ??  ??  CMPS??  ??  ??  \
??  ??  ??  CMPU??  ??  ??  ??  ??  ??  ??  ??  CMPS??  ??  ??  \
??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  \
??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  \
??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  \
??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  ??  \
";

int adr[]=
{0,1,1,0,0,1,0,0,0,0,0,1,0,0,0,0
,6,6,1,1,1,1,2,2,1,1,3,1,3,1,1,1
,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
,4,4,4,4,1,1,1,1,1,1,1,1,1,1,1,1
,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4
,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5
,3,3,3,3,3,3,3,3,3,3,3,3,3,2,3,1
,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4
,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5
,3,3,3,3,3,3,3,1,3,3,3,3,3,1,3,1
,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4
,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5

,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
,1,1,1,3,1,1,1,1,1,1,1,1,3,1,3,1
,1,1,1,0,1,1,1,1,1,1,1,1,0,1,0,0
,1,1,1,4,1,1,1,1,1,1,1,1,4,1,4,4
,1,1,1,5,1,1,1,1,1,1,1,1,5,1,5,5
,1,1,1,1,1,1,1,1,1,1,1,1,1,1,3,1
,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0
,1,1,1,1,1,1,1,1,1,1,1,1,1,1,4,4
,1,1,1,1,1,1,1,1,1,1,1,1,1,1,5,5

,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
,1,1,1,3,1,1,1,1,1,1,1,1,3,1,1,1
,1,1,1,0,1,1,1,1,1,1,1,1,0,1,1,1
,1,1,1,4,1,1,1,1,1,1,1,1,4,1,1,1
,1,1,1,5,1,1,1,1,1,1,1,1,5,1,1,1
,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
};

int tbpshs[8]={9,7,8,10,0,1,2,4};
int tbpshu[8]={9,7,8,10,0,1,3,4};


void ini6809()
{
}

int afreg(int i)
{
  if (i==-1)	{
    putchar('D');
    return 1;
  }
  if (i==0)	{
    putchar('X');
    return 1;
  }
  if (i==1)	{
    putchar('Y');
    return 1;
  }
  if (i==2)	{
    putchar('U');
    return 1;
  }
  if (i==3)	{
    putchar('S');
    return 1;
  }
  if (i==4)	{
    printf("PC");
    return 2;
  }
  if (i==7)	{
    putchar('A');
    return 1;
  }
  if (i==8)	{
    putchar('B');
    return 1;
  }
  if (i==9)	{
    printf("CC");
    return 2;
  }
  if (i==10)	{
    printf("DP");
    return 2;
  }
  printf("??");
  return 2;
}

int afdep(long k, int i)
{
  int	d=0;
  if (k<0)	{
    putchar('-');
    d=1;
    k=-k;
  }
  printhx(k,i);
  return d+i;
}

int des6809(long k, char *c)
{
  int	d;
  int	r;
  int	i;
  long	x;
  int	esp;
  int	po;
  int	cd;
  int	reg;
  
  r=c[0]&255;
  if (r==0x10) r=(c[1]&255)+256;
  if (r==0x11) r=(c[1]&255)+512;
  for(i=0;i<4;i++) putchar(mne[r*4+i]);
  printf("  ");
  d=taille[r];
  esp=0;
  
  switch(adr[r])	{
  case 0:			/*	direct	*/
    printf(".");
    printhx(c[1],2);
    esp=3;
    break;
  case 1:			/*	inherent	*/
    if ((r==0x1e)||(r==0x1f))
      {
        esp+=afreg(((c[1]>>4)&15)-1);
        putchar(',');
        esp+=afreg((c[1]&15)-1)+1;
      }
    if (r==0x3c)
      {
        putchar('#');
        printhx(c[1],2);
        esp+=3;
      }
    if ((r==0x34)||(r==0x35))
      {
        int	flag=0;
        
        for(i=0;i<8;i++)
          if (c[1]&(1<<i))
            {
              if (flag) putchar(',');
              esp+=afreg(tbpshs[i])+flag;
              flag=1;
            }
      }
    if ((r==0x36)||(r==0x37))
      {
        int	flag=0;
        
        for(i=0;i<8;i++)
          if (c[1]&(1<<i))
            {
              if (flag) putchar(',');
              esp+=afreg(tbpshu[i])+flag;
              flag=1;
            }
      }
    
    break;
  case 2:			/*	relatif		*/
    if (d==2)	x=c[1];
    else	x=c[d-2]*256+(c[d-1]&255);
    x+=k+offs+d;
    printf(">");
    printhx(x,4);
    esp=5;
    follow=(x-offs)&0xffff;
    break;
  case 3:			/*	immediat	*/
    printf("#");
    if (d==2)	{
      printhx(c[1],2);
      esp-=2;
    }
    else	printhx(c[d-2]*256+(c[d-1]&255),4);		
    esp+=5;
    break;
  case 4:			/*	indirect	*/
    po=c[d-1];
    cd=po&31;
    reg=(po>>5)&3;
    if (po&0x80)
      {
        if (cd&16) putchar('[');
        switch (cd&15)	{
        case 0 :
          putchar(',');
          esp+=afreg(reg)+2;
          putchar('+');
          break;
        case 1:
          putchar(',');
          esp+=afreg(reg)+3;
          printf("++");
          break;
        case 2 :
          printf(",-");
          esp+=afreg(reg)+2;
          break;
        case 3:
          printf(",--");
          esp+=afreg(reg)+3;
          break;
        case 4:
          putchar(',');
          esp+=afreg(reg)+1;
          break;
        case 5:
          printf("B,");
          esp+=afreg(reg)+2;
          break;
        case 6:
          printf("A,");
          esp+=afreg(reg)+2;
          break;
        case 8:
          esp+=afdep(c[d],2);
          putchar(',');
          esp+=afreg(reg)+1;
          d++;
          break;
        case 9:
          esp+=afdep(c[d]*256+(c[d+1]&255),2);
          putchar(',');
          esp+=afreg(reg)+1;
          d+=2;
          break;
        case 11:
          printf("D,");
          esp+=afreg(reg)+2;
          break;
        case 12:
          reg=4;
          esp+=afdep(c[d],2);
          putchar(',');
          esp+=afreg(reg)+1;
          d++;
          break;
        case 13:
          reg=4;
          esp+=afdep(c[d]*256+(c[d+1]&255),2);
          putchar(',');
          esp+=afreg(reg)+1;
          d+=2;
          break;
        case 15:
          printhx(c[d]*256+(c[d+1]&255),4);
          esp+=4;
          d+=2;
          break;
          
        default:
          break;	}
        if (cd&16)	{
          putchar(']');
          esp+=2;
        }
      }
    else
      {
        if (cd&16)	{
          putchar('-');
          cd=-cd;
          esp++;
        }
        printhx(cd&31,2);
        putchar(',');
        esp+=afreg(reg)+2;
      }
    break;
  case 5 :
    putchar('>');
    x=c[d-2]*256+(c[d-1]&255);
    printhx(x,4);
    esp+=5;
    follow=(x-offs)&0xffff;
    break;
  default:
    break;	}
  for(i=esp;i<18;i++) putchar(' ');
  return d;
}

