
#ifndef MAIN_H
#define MAIN_H

#define DEBUG

const char *USAGE = "\nUsage: filter [OPTIONS] <input image> <output image>\nOPTIONS:\n  -k <dim> sets filter kernel dimension, if blank kernel will be of size 3\n\n";

argS *ap;
pthread_t *tp;
int nT, nTm1, tN;
int id;

struct timeval cBegin, cEnd;
long cSecs, cmSecs;
double cElapsed;
int i;
char c;

int ksize, tmp;
char *infile = NULL, *outfile = NULL;
FILE *fin, *fout;
int width, height, maxval;
unsigned char *image, *out;
double *kernel, *kp;

#endif