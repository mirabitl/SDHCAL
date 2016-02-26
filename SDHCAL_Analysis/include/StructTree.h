#ifndef _STRUCTTREE_H_
#define _STRUCTTREE_H_
#include<stdint.h>

typedef struct { 
  double xhit[61]; 
  double yhit[61]; 
  float  ax; 
  float  bx; 
  float  ay; 
  float  by; 
  float  chi2; 
  int32_t    npoint; 
  int32_t    idx; 
  uint32_t    eventid;
  uint8_t nhit0[61]; 
  uint8_t nhit1[61]; 
  uint8_t nhit2[61]; 
} track_t; 

typedef struct { 
  float  x; 
  float  y; 
  float  dx; 
  float  dy; 
  float  z; 
  int32_t    tkid; 
  int32_t    id; 
  int32_t    eventid; 
  int32_t    runid; 
  int32_t    chamber;
}  point_t;

typedef struct {
  float  X;
  float  Y;
  float  dX;
  float  dY;

  int32_t id;
  int32_t pointid;
} cluster_t;


typedef struct {
  float  X;
  float  Y;
  float  Z;
  int8_t   I,J;
  int32_t id;
  int32_t clusterid;
  int8_t dif;
  int8_t amplitude;
  uint8_t difI,difJ,asic,channel;


} hit_t;

typedef struct { 
  uint64_t    bcid;
  uint64_t    ib1;
  uint32_t    time;
  uint32_t    eventid;
  uint32_t    showers;
  uint32_t    gtc;
  double     xm[3];
  double     lambda[3];
  double     v1[3];
  double     v2[3];
  double     v3[3];
  double     xb1,yb1,maxlb1;
  double     n9,n25,rbs,rbt;
  

  double     rxm[3];
  double     rlambda[3];
  double     rv1[3];
  double     rv2[3];
  double     rv3[3];
  double     corr0[60];
  double     corr1[60];
  double     corr2[60];
  double     rncor[3];
  uint32_t    idx; 

  uint16_t    nhit[3];
  uint16_t    rnhit[3];


  uint16_t    plan0[60];
  uint16_t    plan1[60];
  uint16_t    plan2[60];

  uint8_t    firstplan;
  uint8_t    lastplan;
  uint8_t    np1,fp1,lp1;
  double     fd[4];
  uint16_t   NH0[8];
  uint16_t   NH1[8];
  uint16_t   NH2[8];
  uint16_t   NH[8];

  uint32_t   nc[3];
  uint32_t   ne[3];
  uint32_t   nm[3];

  uint32_t   namas,ngood,nhitafterlast;
  double     zfirst,zlast,tklen;
  
  uint16_t m0,e0,c0,i0;
  uint16_t m1,e1,c1,i1;
  uint16_t m2,e2,c2,i2;
} shower_t;

 

typedef struct { 
  uint32_t    idx;
  uint64_t    bcid;
  uint64_t    bsplan;
  uint64_t    bsiplan;
  uint64_t    bsrplan;
  uint32_t    iclu,rclu,iplan,rplan,aplan;
  double      energy,x0,y0;
  uint32_t    run; 
  uint32_t    event; 
  uint32_t    gtc;
  uint32_t    dtc;
  uint32_t    time;
  double    tospill;
  uint32_t    npoint;
  uint32_t    allpoints;
  uint16_t    ntrack;
  uint16_t    allshowers;
  uint16_t    showers;
  uint8_t     type;
  uint8_t     fpi,lpi;
  uint16_t m0,e0,c0,i0;
  uint16_t m1,e1,c1,i1;
  uint16_t m2,e2,c2,i2;
  uint16_t sm0,se0,sc0,si0;
  uint16_t sm1,se1,sc1,si1;
  uint16_t sm2,se2,sc2,si2;
  double tracklength;
} event_t;

#endif
