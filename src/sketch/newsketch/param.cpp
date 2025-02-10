#include "param.h"
double lim = 0.005;
double weaklim = 0.01;
double p = 0.1;
double outv = 0.5;
int threshold = 1;
double chklim = 0.05;
int hash1 = 2;

void load_param(struct Param x) {
    chklim = x._chklim;
    outv = x._outv;
    threshold = x._threshold;
    p = x._p;
    lim = x._lim;
    weaklim = x._weaklim;
    hash1 = x._hash1;
}