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

  monitor.c : moniteur de l'emulateur

*/

#include <string.h>
#include "emuto7.h"
#include "monitor.h"
#include "repert.h"
#include "video.h"

extern int nbwait;
char    buf[sizebuf];
long    leng;
long    ind;
long    offs;
int     nbo;
int     mode;
long    follow;
int     regon;
int     asmon;
int     saveon;

char    saisie[256];
int     mot[16];
int     nbmot;

int     modegraph;

char    voc[]="q param load d e save merge size fill offset check n find \
               replace vocab m6809 go reset k7 seek saveon saveoff copyto7 \
               convbasic cat dir cd pwd speed ";
int     com[100];
int     nbword;
FILE    *fl;

void printhx(long k,int n)
{
  char  c;
  int   i;

  for(i=n-1;i>=0;i--)
    {
      c=48+((k>>(i*4))&15);
      if (c>57) c+=7;
      printf("%c",c);
    }
}

void fprinthx(FILE *f,long k,int n)
{
  char  c;
  int   i;

  for(i=n-1;i>=0;i--)
    {
      c=48+((k>>(i*4))&15);
      if (c>57) c+=7;
      fprintf(f,"%c",c);
    }
}

void printad(long k)
{
  printhx(k+offs,8);
}

int gethx(char *c, long *l)
{
  char  a;
  long  k;
  int   s,i,n;
  k=0;
  s=i=0;
  while(c[i]!=0)
    {
      a=c[i];
      if((i==0)&&(a=='-')) s=1;
      else      {
        n=-1;
        if ((a>='0')&&(a<='9')) n=a-'0';
        if ((a>='A')&&(a<='F')) n=10+a-'A';
        if ((a>='a')&&(a<='f')) n=10+a-'a';
        if (n!=-1) k=16*k+n;
        else return -1;
      }
      i++;
    }
  if (s) k=-k;
  *l=k;
  return 0;
}

int getad(char *c, long *l)
{
  long  k;
  int   i;
  i=gethx(c,&k);
  *l=k-offs;
  return i;
}

void inimonitor(char *name)
{
  int   i,j,f;

  leng=0;
  ind=0;
  offs=0;
  nbo=1;
  printf("\n\n          Emulateur TO7-70 . version 1.5\n");
  printf("               par Sylvain HUET\n\n");
  printf("Principales fonctions :\n");
  printf("-go          : lancement de l'emulateur\n");
  printf("-reset       : bouton 'reset' du TO7\n");
  printf("-k7 [nom]    : changement de fichier k7\n");
  printf("-load [nom]  : chargement d'une cartouche memo7\n");
  printf("-seek [num]  : positionnement de la bande\n");
  printf("-speed [num] : reglage de la boucle de ralentissement\n");
  printf("-q           : quitter\n\n");
  printf("Lors de l'emulation :\n");
  printf("-bouton droit  : retour au moniteur\n");
  printf("-bouton milieu : deplacement du stylo optique sur l'ecran\n");
  printf("-bouton gauche : interrupteur du stylo optique\n\n");

  i=j=f=0;
  while(voc[i]!=0)
    {
      if (voc[i]!=32)
        {
          if (f==0) com[j++]=i;
          f=1;
        }
      else
        {
          voc[i]=0;
          f=0;
        }
      i++;
    }
  nbword=j;
  mode=1;
  regon=0;
  asmon=0;
  saveon=0;
  ini6809();
  inito7(name);
}

void decoupe(void)
{
  char  c;
  int   i,j,f;

  i=j=f=0;
  while((c=getchar())!=10)
    {
      if (c!=32)
        {
          saisie[i]=c;
          if (f==0) mot[j++]=i;
          f=1;
        }
      else
        {
          saisie[i]=0;
          f=0;
        }
      i++;
    }
  saisie[i]=0;
  nbmot=j;
}

int numcom(int i)
{
  int   j;
  long  k;

  for(j=0;j<nbword;j++)
    if (!strcmp(&saisie[i],&voc[com[j]])) return j;
  if (getad(&saisie[i],&k)==0) return -1;
  return -2;
}

int vocab(int i)
{
  int   j;
  for(j=0;j<nbword;j++)
    printf("%s\n",&voc[com[j]] );

  return(0); /* seg_/sreg: un warning de moins */
}

int param(int i)
{
  printf("taille du buffer : ");
  printhx(leng,8);
  printf("\ntaille maximale  : ");
  printhx(sizebuf,8);
  printf("\noffset           : ");
  printhx(offs,8);
  printf("\nindex            : ");
  printad(ind);
  printf("\ntaille edition   : ");
  printhx(nbo,2);
  printf("\n");
  if (mode==0) printf("mode numerique\n");
  if (mode==1) printf("mode 6809\n");
  return 0;
}

int load(int i)
{
  long  k;
  int   c;

  if (i+1>=nbmot)
    {
      printf("incomplet\n");
      return -1;
    }
  printf("chargement de %s\n",&saisie[mot[i+1]]);
  fl=fopen(&saisie[mot[i+1]],"r");
  if (fl==NULL)
    {
      printf("introuvable\n");
      return -1;
    }
  k=0;
  ind=0;
  leng=0;
  while((k<sizebuf)&&((c=fgetc(fl))!=EOF)) buf[k++]=c;
  fclose(fl);
  if (c!=EOF)
    {
      printf("trop long\n");
      return -1;
    }
  leng=k;
  return 0;
}

long afligne(long k)
{
  long  i;
  char  c;

  printad(k);
  printf("  ");

  for(i=k;i<k+16;i++)
    {
      if (i<leng)
        printhx(buf[i],2);
      else
        printf("  ");
      printf(" ");
    }
  printf("   ");
  for(i=k;i<k+16;i++)
    {
      if (i<leng)
        {
          c=buf[i];
          if ((c<32)||(c&0x80)) c='.';
        }
      else
        c=' ';
      printf("%c",c);
    }
  printf("\n");
  if (i<leng) return i;
  else
    return leng;
}

int dump(int i)
{
  long  k,last,indl;
  /* int   j;  inutilise */

  last=ind+512;
  indl=ind;

  if (i+1<nbmot)
    {
      if (!strcmp(&saisie[mot[i+1]],"all"))
        {
          ind=0;
          last=leng;
        }

      if (getad(&saisie[mot[i+1]],&k)==0)
        {
          ind=k;
          last=ind+512;
        }
      if (i+2<nbmot)
                {
                  if (!strcmp(&saisie[mot[i+2]],"ind")) last=indl;
                  if (!strcmp(&saisie[mot[i+2]],"end")) last=leng;
                  if (getad(&saisie[mot[i+2]],&k)==0)   last=k;
                }
    }
  if (last>leng) last=leng;
  while(ind<last) ind=afligne(ind);
  ind=last;
  if (ind>=leng) ind=0;

  return(0); /* seg_/sreg: un warning de moins */
}

int save(int i)
{
  long  deb;
  long  fin;
  long  k;

  if (i+1>=nbmot)
    {
      printf("incomplet");
      return -1;
    }
  deb=0;
  fin=leng;
  if (i+2<nbmot)
    {
      if (!strcmp(&saisie[mot[i+2]],"ind")) deb=ind;
      if (getad(&saisie[mot[i+2]],&k)==0) deb=k;
      if (i+3<nbmot)
        {
          if (!strcmp(&saisie[mot[i+3]],"ind")) fin=ind;
          if (getad(&saisie[mot[i+3]],&k)==0)   fin=k;
        }
    }
  if (fin>leng) fin=leng;
  printf("sauvegarde de %s\n",&saisie[mot[i+1]]);
  fl=fopen(&saisie[mot[i+1]],"w");
  if (fl==NULL)
    {
      printf("impossible\n");
      return -1;
    }
  for(k=deb;k<fin;k++) fputc(buf[k],fl);
  fclose(fl);
  return 0;
}

int merge(int i)
{
  long  k;
  int   c;
  int   op;

  if (i+1>=nbmot)
    {
      printf("incomplet\n");
      return -1;
    }

  if ((i+2>=nbmot)||(getad(&saisie[mot[i+2]],&k)!=0)) k=0;

  op=0;
  if (i+3<nbmot)
    {
      if (!strcmp(&saisie[mot[i+3]],"add")) op=1;
      if (!strcmp(&saisie[mot[i+3]],"sub")) op=2;
      if (!strcmp(&saisie[mot[i+3]],"and")) op=3;
      if (!strcmp(&saisie[mot[i+3]],"or")) op=4;
      if (!strcmp(&saisie[mot[i+3]],"xor")) op=5;
      if (op==0)
        {
          printf("operation inconnue\n");
          return -1;
        }
    }

  printf("importation de %s\n",&saisie[mot[i+1]]);
  fl=fopen(&saisie[mot[i+1]],"r");
  if (fl==NULL)
    {
      printf("introuvable\n");
      return -1;
    }

  while((k<sizebuf)&&((c=fgetc(fl))!=EOF))
    {
      switch (op)
        {
        case 1 :
          buf[k++]+=c;
          break;
        case 2 :
          buf[k++]-=c;
          break;
        case 3 :
          buf[k++]&=c;
          break;
        case 4 :
          buf[k++]|=c;
          break;
        case 5 :
          buf[k++]^=c;
          break;
        default:
          buf[k++]=c;
          break;        }
    }
  fclose(fl);
  if (c!=EOF)
    {
      printf("importation tronquee\n");
      return -1;
    }
  if (k>leng) leng=k;
  return 0;
}

int size(int i)
{
  long  k;

  if (i+1>=nbmot)
    {
      printf("incomplet\n");
      return -1;
    }
  if (getad(&saisie[mot[i+1]],&k)!=0)
    {
      printf("incomplet\n");
      return -1;
    }
  if (k>sizebuf) k=sizebuf;
  if (ind>=k) ind=0;
  leng=k;
  param(i);
}

int fill(int i)
{
  long  k,deb,fin;
  char  motif;

  deb=0;
  fin=leng;
  motif=0;
  if (i+1<nbmot)
    if (gethx(&saisie[mot[i+1]],&k)==0) motif=k;
  if (i+2<nbmot)
    {
      if (!strcmp(&saisie[mot[i+2]],"ind")) deb=ind;
      if (getad(&saisie[mot[i+2]],&k)==0) deb=k;
    }
  if (i+3<nbmot)
    {
      if (!strcmp(&saisie[mot[i+3]],"ind")) fin=ind;
      if (getad(&saisie[mot[i+3]],&k)==0) fin=k;
    }

  if (deb>leng) deb=leng;
  if (fin>leng) fin=leng;
  for(k=deb;k<fin;k++) buf[k]=motif;

  return(0); /* seg_/sreg: encore un warning de moins pour le SAS */
}

int offset(int i)
{
  long  k;

  offs=0;
  if ((i+1<nbmot)&&(gethx(&saisie[mot[i+1]],&k)==0)) offs=k;
  return 0;
}

int check(int i)
{
  long  k,deb,fin,s,v;

  deb=0;
  fin=leng;
  if (i+1<nbmot)
    {
      if (!strcmp(&saisie[mot[i+1]],"ind")) deb=ind;
      if (getad(&saisie[mot[i+1]],&k)==0) deb=k;
    }
  if (i+2<nbmot)
    {
      if (!strcmp(&saisie[mot[i+2]],"ind")) fin=ind;
      if (getad(&saisie[mot[i+2]],&k)==0) fin=k;
    }

  if (deb>leng) deb=leng;
  if (fin>leng) fin=leng;
  s=0;
  for(k=deb;k<fin;k++)
    {
      v=buf[k]&255;
      s+=v;
    }
  printf("checksum : ");
  printhx(s,8);
  printf("\n");
  return 0;
}

int affedit(void)
{
  int   j;
  int   n;

  n=1;
  printad(ind);
  printf("    ");
  if (mode==0)
    {
      for(j=0;j<nbo;j++) printhx(buf[ind+j],2);
      n=nbo;
    }
  if (mode==1) n=des6809(ind,&buf[ind]);
  return n;
}

int interpedit(int d)
{
  long  k,j,l;

  if (nbmot==0)
    {
      ind+=d;
      return 0;
    }
  if (!strcmp(&saisie[mot[0]],"q")) return 1;
  if (!strcmp(&saisie[mot[0]],"r"))
    {
      if (offs!=0)
        {
          printf("impossible : offset non nul\n");
          return 0;
        }
      k=1;
      if (nbmot>1) if (gethx(&saisie[mot[1]],&k)) k=1;
      l=0;
      compt=0;
      quitf=1;
      for(j=0;j<k;j++)
        if (quitf)
          {
            if (j&&asmon)
              {
                affedit();
                printf("\n");
              }
            compt++;
            exe6809(); ind=pc;
            if (saveon) fprinthx(fk7,ind,4);
            if (regon) printreg();
            if (++l==8000)
              {
                refreshv();
                l=0;
              }
          }
      refreshv();
      return 0;
    }
  if (!strcmp(&saisie[mot[0]],"ref"))
    {
      refreshsc();
      return 0;
    }

  if (!strcmp(&saisie[mot[0]],"s"))
    {
      if (nbmot>1)
        {
          if (getad(&saisie[mot[1]],&k)==0) ind=k;
        }
      else ind=follow;
      return 0;
    }
  if (!strcmp(&saisie[mot[0]],"reg"))
    {
      printreg();
      return 0;
    }
  if (!strcmp(&saisie[mot[0]],"regon"))
    {
      regon=1;
      printreg();
      return 0;
    }
  if (!strcmp(&saisie[mot[0]],"regoff"))
    {
      regon=0;
      return 0;
    }
  if (!strcmp(&saisie[mot[0]],"asmon"))
    {
      asmon=1;
      return 0;
    }
  if (!strcmp(&saisie[mot[0]],"asmoff"))
    {
      asmon=0;
      return 0;
    }

  if (!strcmp(&saisie[mot[0]],"set"))
    {
      if (nbmot<3) return 0;
      if (gethx(&saisie[mot[2]],&k)) return 0;
      setreg(&saisie[mot[1]],k);
      printreg();
      return 0;
    }

  if (gethx(&saisie[mot[0]],&k)==0)
    {
      for(j=0;j<d;j++)
        buf[ind+j]=(k>>((d-1-j)*8))&255;
      ind+=d;
      return 0;
    }
  return 0;
}

int edit(int i)
{
  int   q,d;
  long  k;

  q=0;
  if (i+1<nbmot) if (getad(&saisie[mot[i+1]],&k)==0) ind=k;
  while(q==0)
    {
      follow=ind;
      d=affedit();
      printf("    / ");
      fflush(stdout);
      decoupe();
      q=interpedit(d);
      if (ind<0) ind=0;
      if (ind>=leng) ind=0;
    }
  return 0;
}

int setnbo(int i)
{
  long  k;

  if (i+1<nbmot)
    {
      nbo=1;
      if (gethx(&saisie[mot[i+1]],&k)==0) nbo=((k-1)&3)+1;
    }
  mode=0;
  printf("mode numerique\n");
  return 0;
}

int trouve(long k, long i)
{
  int   j;

  for(j=0;j<nbo;j++)
    if ((255&(long)buf[i+j])!=((k>>((nbo-1-j)*8))&255)) return 0;
  return 1;
}

void rempla(long k, long i)
{
  int   j;

  for(j=0;j<nbo;j++)
    buf[i+j]=(k>>((nbo-1-j)*8))&255;
}

int find(int i)
{
  long  k,j;
  int   f;

  f=0;
  if (i+1>=nbmot)
    {
      printf("incomplet\n");
      return 1;
    }
  if (gethx(&saisie[mot[i+1]],&k)!=0)
    {
      printf("incomplet\n");
      return 1;
    }
  j=ind;
  if (i+2<nbmot) if (!strcmp(&saisie[mot[i+2]],"all"))
    {
      j=0;
      f=1;
    }
  do
    {
      while((j<=leng-nbo)&&(trouve(k,j)==0)) j++;
      if (j>leng-nbo)
        {
          if (f==0) printf("pas trouve\n");
          return 1;
        }
      ind=j;
      printf("trouve en ");
      printad(ind);
      printf("\n");
      j++;
    }
  while(f);
}

int replace(int i)
{
  long  k,j,l;
  int   f;

  f=0;
  if (i+2>=nbmot)
    {
      printf("incomplet\n");
      return 1;
    }
  if (  (gethx(&saisie[mot[i+1]],&l)!=0)
      ||(gethx(&saisie[mot[i+2]],&k)!=0)  )
    {
      printf("incomplet\n");
      return 1;
    }
  j=ind;
  if (i+3<nbmot) if (!strcmp(&saisie[mot[i+3]],"all"))
    {
      j=0;
      f=1;
    }
  do
    {
      while((j<=leng-nbo)&&(trouve(l,j)==0)) j++;
      if (j>leng-nbo)
        {
          if (f==0) printf("pas trouve\n");
          return 1;
        }
      ind=j;
      printf("remplace en ");
      printad(ind);
      printf("\n");
      rempla(k,j);
      j+=nbo;
    }
  while(f);
}

int interp(int i)
{
  int   m;
  int   q;
  int   j;
  long  k;
  char  c;

  q=0;
  if (i>=nbmot) m=-2;
  else  m=numcom(mot[i]);
  switch (m)    {
  case -1:                      /*      index   */
    getad(&saisie[mot[i]],&ind);
    break;
  case 0:                       /*      quit    */
    printf("quitter (o/n)? ");
    fflush(stdout);
    if ((c=getchar())=='o')
      {
        printf("\n\n");
        q=1;
      }
    while(c!=10) c=getchar();
    break;
  case 1:                       /*      param   */
    param(i);
    break;
  case 2:                       /*      load    */
    load(i);
    break;
  case 3:                       /*      dump    */
    dump(i);
    break;
  case 4:                       /*      edit    */
    edit(i);
    break;
  case 5:                       /*      save    */
    save(i);
    break;
  case 6:                       /*      merge   */
    merge(i);
    break;
  case 7:                       /*      cut     */
    size(i);
    break;
  case 8:                       /*      fill    */
    fill(i);
    break;
  case 9:                       /*      offset  */
    offset(i);
    break;
  case 10:              /*      check   */
    check(i);
    break;
  case 11:              /*      n       */
    setnbo(i);
    break;
  case 12:              /*      find    */
    find(i);
    break;
  case 13:              /*      replace */
    replace (i);
    break;
  case 14:              /*      vocab   */
    vocab(i);
    break;
  case 15:              /*      m6809   */
    mode=1;
    printf("mode 6809\n");
    break;
  case 16:              /*      emulateur to7 */
    printf("emulation to7 en cours\nbouton 3 pour stopper\n");
    j=bouclex();
    break;
  case 17:              /*      reset pc        */
    setreg("pc",0xf98d);
    printreg();
    break;
  case 18:              /*      change k7       */
    if (i+1<nbmot) newcass(&saisie[mot[i+1]]);
    break;
  case 19:              /*      seek k7         */
    if (i+1<nbmot) if (gethx(&saisie[mot[i+1]],&k)==0)
      avanck7(k);
    break;
  case 20:
    saveon=1;
    break;
  case 21:
    saveon=0;
    break;
  case 22:              /*      copy to7->pc    */
    copyto7();
    break;
  case 23:              /*      conversion basic */
    convbasic();
    break;
  case 24:              /*      catalogue       */
    catdsk();
    break;
  case 25:                /*    dir   */
    displaydir();
    break;
  case 26:                /*      cd   */
    if (i+1<nbmot) changedir(&saisie[mot[i+1]]);
    break;
  case 27:                /*      pwd  */
    displaypath();
    break;
  case 28:
    if (i+1<nbmot)
      {
        if (gethx(&saisie[mot[i+1]],&k)==0) nbwait=k;
      }
    else printf("ralentissement=%x\n",nbwait);
    break;
  default:
    printf("???\n");
    break;      }
  return q;

}

int importrom(void)
{
  int i;

  for(i=0xe000;i<0xe7c0;i++) buf[i]=0x39;
  for(i=0xe7c0;i<0xe800;i++) buf[i]=0;
  buf[0xe7c3]=-127; /* 0x81; seg_/sreg: encore un warn de moins */

  strcpy(saisie,"merge romto770 e800");
  saisie[5]=saisie[5+9]=0;
  mot[0]=0; mot[1]=6; mot[2]=15;
  nbmot=3;
  interp(0);

  buf[0xfbd3]=0x02; buf[0xfbd4]=0x39;
  buf[0xf07c]=0x02; buf[0xf07d]=0x39;

  strcpy(saisie,"load memo7/basic");
  saisie[4]=0;
  mot[0]=0; mot[1]=5;
  nbmot=2;
  interp(0);

  return 0;
}

int main(int argc,char **argv)
{
  int   q=0;
  int   i;
  char  *name;

  modegraph=1;
  flto7=0;
  endmem=0xdfff;
  name="keyto7";
  for(i=0;i<argc;i++)
    {
      if(!strcmp(argv[i],"-k")) name=argv[i+1];
      if(!strcmp(argv[i],"-gray")) modegraph=2;
      if(!strcmp(argv[i],"-mono")) modegraph=0;
      if(!strcmp(argv[i],"-h"))
        {
          printf("options :\n");
          printf(" -k [nom] : clavier (keyto7 par defaut)\n");
          printf(" -mono : ecran mono\n");
          printf(" -gray : palette grise\n");
          exit(1);
        };
    }
  inimonitor(name);

  importrom();
  nbwait=0;

  while(q==0)
    {
      printf("ok\n");
      printad(ind);
      printf(" >");
      fflush(stdout);
      decoupe();
      q=interp(0);
    }
  return 0;
}

