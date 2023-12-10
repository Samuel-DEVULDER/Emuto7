/* Fichier de configuration */

/* mettre DECALPHA a 1 si vous utilisez une station DecAlpha */

#define DECALPHA 0

#ifdef __SASC
#undef __SASC
#define __SASC    1
#define __ASMSASC 1
#endif
