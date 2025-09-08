#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define __USE_GNU
#include <unistd.h>
#include <sys/time.h>
#include <string.h>

#include <pthread.h>

typedef struct argS {
  unsigned char *img;
  unsigned char *out;
  double *kernel;
  int ksize;
  int width;
  int height;
  int start;
  int end;
} argS;

static void *filter (void *arg) {
  argS *ap = (argS *) arg;
  unsigned char *in = (unsigned char *) ap->img;
  unsigned char *out = (unsigned char *) ap->out;
  double *kernel = (double *) ap->kernel;
  int ksize = (int) ap->ksize;
  int w = (int) ap->width;
  int h = (int) ap->height;
  int start = (int) ap->start;
  int end = (int) ap->end;

  double sum;
  int yy, xx;
  int r = ksize / 2;

  for (int y = start; y < end; y++) {  // sweep the image
    for (int x = 0; x < w; x++) {
      sum = 0;
      for (int dy = -r; dy <= r; dy++) {    // apply kernel
        for (int dx = -r; dx <= r; dx++) {
          yy = y + dy;
          xx = x + dx;
          // mirror image borders
          if (yy < 0) yy = 0;
          if (yy >= h) yy = h - 1;
          if (xx < 0) xx = 0;
          if (xx >= w) xx = w - 1;

          double kval = *(kernel + ksize * (dy + r) + (dx + r));
          sum += *(in + yy * w + xx) * kval;
        }
      }

      if (sum < 0) sum = 0;
      if (sum > 255) sum = 255;
      *(out + y * w + x) = (unsigned char) sum;
    }
  }
  

  return NULL;
}

int main (int argc, char **argv) {
  #include <filter.h>
  if (argc != 3 && argc != 5) {
    fprintf (stderr, "%s", USAGE);
    exit (1);
  }
  ksize = 3;
  while ((c = getopt (argc, argv, "k:")) != -1)
    switch (c) {
      case 'k':
        if (sscanf (optarg, "%d", &ksize) != 1) {
          fprintf (stderr, "Set the kernel dimensions.\n");
          exit (1);
        }
        break;
      default:
        fprintf (stderr, "%s", USAGE);
        exit (1);
    }
  
  if (ksize < 3) {
    fprintf (stderr, "Kernel size must be >= 3\n");
    exit (1);
  }

  // enforce odd kernel size
  if ((ksize % 2) == 0) ksize += 1;

  // create kernel
  if ((kernel = malloc (ksize * ksize * sizeof (double))) == 0) {
    fprintf (stderr, "Cannot allocate (double) kernel");
    exit (1);
  }
  // simple blur
  for (i = 0, kp = kernel; i < ksize * ksize; i++, kp++) {
    *kp = 1.0f / (ksize * ksize);
  }
  
  infile = argv[optind];
  outfile = argv[optind + 1];

  if ((fin = fopen (infile, "r")) == NULL) {
    fprintf (stderr, "Cannot open file %s\n", infile);
    exit (1);
  }

  // check pgm header
  char header[3];   // should be 'P''2''\0'
  if (fscanf (fin, "%2s", header) != 1) {
    fprintf (stderr, "Cannot find PGM header\n");
    exit (1);
  }
  if (header[0] != 'P' || (header[1] != '2' && header[1] != '5')) {
    fprintf (stderr, "Only ASCII PGM (P2) and bynary PGM (P5) header is supported, is %s\n", header);
    exit (1);
  }

  // skip comments
  while ((i = fgetc (fin)) == '#') {
    while (fgetc (fin) != '\n');
  }
  ungetc (i, fin);
  // read width height and max value
  if (fscanf (fin, "%d %d %d", &width, &height, &maxval) != 3) {
    fprintf (stderr, "Bad header data\n");
    exit (1);
  }
  fgetc (fin); // skip new line after maxval

  // allocate images
  if ((image = malloc (width * height * sizeof (unsigned char))) == 0) {
    fprintf (stderr, "Cannot allocate (char) image");
    exit (1);
  }
  if ((out = malloc (width * height * sizeof (unsigned char))) == 0) {
    fprintf (stderr, "Cannot allocate (char) out");
    exit (1);
  }

  if (header[1] == '5') {
    if (fread (image, 1, width * height, fin) != width * height) {
      fprintf (stderr, "Error reading file\n");
      exit (1);
    }
  } else {
    for (i = 0; i < width * height; i++) {
      if (fscanf (fin, "%d", &tmp) != 1) {
        fprintf (stderr,"Bad pixel data\n"); 
        exit (1);
      }
      image[i] = (unsigned char) tmp;
    }  
  }

  fclose (fin);

  // number of threads should be a divisor of h
  // as to assign tN rows to each thread
  nT = sysconf (_SC_NPROCESSORS_ONLN); // number of online threads
  for (nT = (nT < height ? nT : height); height % nT != 0; nT--);
  nTm1 = nT - 1;
  tN = (int) height / nT;    // number of rows per thread

  pthread_t thread[nTm1];
  argS arg[nT];
  
  // fill arg vector
  for (id = 0, ap = arg; id < nT; id++, ap++) {
    ap->img = image;
    ap->out = out;
    ap->kernel = kernel;
    ap->ksize = ksize;
    ap->width = width;
    ap->height = height;
    ap->start = id * tN;
    ap->end = id * tN + tN;
  }

  gettimeofday (&cBegin, 0);
  for (id = 1, tp = thread, ap = arg + 1; id < nT; ++id, tp++, ap++) {
    pthread_create (tp, NULL, filter, ap);
  }
  filter (arg);
  for (id = 1, tp = thread; id < nT; ++id, tp++) {
    pthread_join (*tp, NULL);
  }
  gettimeofday (&cEnd, 0);

  cSecs = cEnd.tv_sec - cBegin.tv_sec;
  cmSecs = cEnd.tv_usec - cBegin.tv_usec;
  cElapsed = cSecs * 1e6 + cmSecs;

  printf ("ksize=%d time_us=%.3f\n", ksize, cElapsed);

  // save output to file
  if ((fout = fopen (outfile, "w")) == NULL) {
    fprintf (stderr, "Cannot open file %s\n", outfile);
    exit (1);
  }

  fprintf (fout, "%s\n%d %d\n%d\n", header, width, height, maxval);
  if (header[1] == '5') {
    fwrite (out, 1, width * height, fout);
  } else {
    for (i = 0; i < width * height; i++) {
      fprintf (fout, "%d ", out[i]);
      if ((i + 1) % width == 0) {
        fprintf (fout, "\n");
      } 
    }
  }
  
  fclose (fout);

  return 0;
}
