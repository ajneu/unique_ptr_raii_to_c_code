#ifndef MYTYPE_H
#define MYTYPE_H

#ifdef __cplusplus
extern "C" {
#endif

struct Mytype {
  int i;
};

// functions  
void  free_Mytype(struct Mytype  *p);
void alloc_Mytype(struct Mytype **p, double x);

  
#ifdef __cplusplus
}
#endif
  
#endif
