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

  copyto7.c : routines de lecture du format disque to7

*/

#include "monitor.h"

char    *taf;
char    *cat;
FILE    *flto;

void copybl(int bl, int endbl)
{
  /* long  k; inutilise */
  int   nbsec,nblast;
  char  *pntr;
  int   nbl;
  int   i,j;

  while(bl<0xc1)
    {
      pntr=&buf[bl*2048];
      nbsec=8;
      nblast=255;

      nbl=taf[bl]&255;
      if (nbl>0xc0)     {
        nbsec=nbl-0xc0;
        nblast=endbl;
      }
      for(i=0;i<nbsec-1;i++)
        {
          for(j=0;j<255;j++) fputc(*(pntr++),flto);
          pntr++;
        }
      for(j=0;j<nblast;j++) fputc(*(pntr++),flto);
      bl=nbl;
    }
}

void copyto7()
{
  char  name[13];
  int   i;
  int   fl;
  char  c;

  taf=&buf[0x14101];
  cat=&buf[0x14200];

  while((*cat)!=-1)
    {
      if (*cat)
        {
          for(i=0;i<8;i++) name[i]=((cat[i]==' ')?'_':cat[i]);
          name[8]='.';
          for(i=0;i<3;i++) name[9+i]=((cat[8+i]==' ')?'_':cat[8+i]);
          name[12]=0;
          printf("%s (o/n)? ",name);
          fl=0;
          while((c=getchar())!=10) if (c=='o') fl=1;

          if ((fl)&&(flto=fopen(name,"w")))
            {
              copybl(cat[13]&255,cat[15]&255);
              fclose(flto);
            }
        }
      cat+=32;
    }

}

void convbasic()
{
  long  fin=((buf[1]<<8)+(buf[2]&255))&0xffff;
  long  ind=3;
  long  newind;
  long  k;

  while(ind<fin)
    {
      newind=(ind+(buf[ind]<<8)+(buf[ind+1]&255))&0xffff;
      k=(newind+0x65f2)>>8;
      buf[ind]=k;
      buf[ind+1]=newind+0x65f2;
      ind=newind;
    }
}

void catdsk()
{
  char  name[13];
  int   i;
  /* int   fl; inutilise */
  /* char  c;  inutilise */

  taf=&buf[0x14101];
  cat=&buf[0x14200];

  while((*cat)!=-1)
    {
      if (*cat)
        {
          for(i=0;i<8;i++) name[i]=((cat[i]==' ')?'_':cat[i]);
          name[8]='.';
          for(i=0;i<3;i++) name[9+i]=((cat[8+i]==' ')?'_':cat[8+i]);
          name[12]=0;
          printf("%s\n",name);
        }
      cat+=32;
    }

}
