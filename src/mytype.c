#include <stdio.h>
#include <stdlib.h>
#include "mytype.h"

void alloc_Mytype(struct Mytype **p)
{
  printf("allocating Mytype\n");
  *p = malloc(sizeof (struct Mytype));
  if (! (*p)) {
    fprintf(stderr, "No more memory\n");
    exit(1);
  }
  (*p)->i = 42;
}

void free_Mytype(struct Mytype *p)
{
  printf("freeing Mytype\n");
  free(p);
}



/*
struct Mytype *malloc_interface_Mytype()
{
  struct Mytype *p = malloc(sizeof p);
  if (!p) {
    fprintf(stderr, "No more memory\n");
    exit(1);
  }    
  return p;
}
*/
