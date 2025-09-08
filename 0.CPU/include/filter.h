
#ifndef MAIN_H
#define MAIN_H

const char *USAGE = "\nUsage: filter [OPTIONS] <input image> <output image>\nOPTIONS:\n  -k <dim> sets filter kernel dimension, if blank kernel will be of size 3\n\n";

struct timeval cBegin, cEnd;
long cSecs, cmSecs;
double cElapsed;

char c;
int n;
int i;

int ksize, tmp;
char *infile = NULL, *outfile = NULL;
FILE *fin, *fout;
int width, height, maxval;
unsigned char *image, *out;
double *kernel, *kp;

#endif
