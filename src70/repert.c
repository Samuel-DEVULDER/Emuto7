/* routines de gestion de repertoire*/

/* Legeres adaptations pour l'amiga par Samuel Devulder en 07/97. */

#include <stdio.h>
#include <stdlib.h>
#ifndef __SASC
#include <unistd.h>
#endif
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/dir.h>

#include "config.h"
#include "repert.h" /* seg_/sreg: faut pas l'oublie lui !!! */

#if DECALPHA
#include <dirent.h>
#endif

#if __SASC
#include <dirent.h>
#endif

void fillspace(char *name, int size)
{
  int i;

  for(i=strlen(name);i<size;i++) putchar(' ');
}


void displaydir(void)
{
  DIR *d;

#if DECALPHA
  struct dirent *dp;
#else
  struct direct *dp;
#endif
  struct stat buf;
  char nam[256];

  if(d=opendir(".")) /* sam: unix path convention */
    {
      while((dp=readdir(d))!=NULL)
        {
          strcpy(nam,"./");
          strcat(nam,dp->d_name);
          stat(nam,&buf);
          if ((buf.st_mode&S_IFMT)==S_IFDIR)
            {
              fillspace(dp->d_name,14);
              printf("<%s>",dp->d_name);
            }
        }
      printf("\n");
      rewinddir(d); /* sam, was: seekdir(d,0); */
      while((dp=readdir(d))!=NULL)
        {
          strcpy(nam,"./");
          strcat(nam,dp->d_name);
          stat(nam,&buf);
          if ((buf.st_mode&S_IFMT)!=S_IFDIR)
            {
              fillspace(dp->d_name,16);
              printf("%s",dp->d_name);
            }
        }
      printf("\n");
    }
  else if(d=opendir("")) /* sam: amiga path convention */
    {
      while((dp=readdir(d))!=NULL)
        {
          strcpy(nam,"");
          strcat(nam,dp->d_name);
          stat(nam,&buf);
          if ((buf.st_mode&S_IFMT)==S_IFDIR)
            {
              fillspace(dp->d_name,14);
              printf("<%s>",dp->d_name);
            }
        }
      printf("\n");
      rewinddir(d); /* sam, was: seekdir(d,0); */
      while((dp=readdir(d))!=NULL)
        {
          strcpy(nam,"");
          strcat(nam,dp->d_name);
          stat(nam,&buf);
          if ((buf.st_mode&S_IFMT)!=S_IFDIR)
            {
              fillspace(dp->d_name,16);
              printf("%s",dp->d_name);
            }
        }
      printf("\n");
    }
}


void changedir(char *path)
{
  chdir(path);
}

void displaypath(void)
{
  char name[256];

  getcwd(name,256);
  printf("%s\n",name);
}


