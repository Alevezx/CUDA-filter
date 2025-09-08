
#ifndef MAIN_H
#define MAIN_H

const int ms2us = 1e3;

const char *USAGE = "\nUsage: filter [OPTIONS] <input image> <output image>\nOPTIONS:\n  -k <dim> sets filter kernel dimension, if blank kernel will be of size 3\n\n";
char c;

cudaError_t e1, e2, e3, error;
cudaDeviceProp properties;
int nDev, device, maxTpB;
cudaEvent_t start, stop;
float gElapsed, fcElapsed;

int ksize, tmp, i;
int side, blocks;
char *infile = NULL, *outfile = NULL;
FILE *fin, *fout;
int width, height, maxval;
unsigned char *image, *out;
double *kernel, *kp;

#endif
