#include <stdio.h>

#ifndef DEBUG_H
#define DEBUG_H


#define DEBUGS(s) fprintf(stderr, "DEBUG: %s\n", s)
#define DEBUGN(s)  fprintf(stderr, "DEBUG: %d\n", s)
#define DEBUGSN(d, s)  fprintf(stderr,"DEBUG: %s: %d\n",d,s)
#define DEBUGSS(d, s) fprintf(stderr,"DEBUG: %s: %s\n",d, s)

#endif
