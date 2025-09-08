
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <unistd.h>
#include <cuda_runtime.h>

using namespace std;

__global__ void filter (unsigned char *in, unsigned char *out, double *kernel,
                        int ksize, int w, int h) {
  extern __shared__ double kernel_s[];

  int x = blockIdx.x * blockDim.x + threadIdx.x;
  int y = blockIdx.y * blockDim.y + threadIdx.y;
  // check pixel existence
  if (x >= w || y >= h) return;

  for (int i = x; i < ksize * ksize; i += blockDim.x * blockDim.y) {
    // copy kernel into shared memory, works even for big kernel sizes
    kernel_s[i] = kernel[i];
  }
  __syncthreads ();
  
  int r = ksize / 2;
  double sum = 0.0;

  // compute convolution
  for (int dy = -r; dy <= r; dy++) {    // apply kernel
    for (int dx = -r; dx <= r; dx++) {
      int yy = y + dy;
      int xx = x + dx;
      // clamp image borders
      if (yy < 0) yy = 0;
      if (yy >= h) yy = h - 1;
      if (xx < 0) xx = 0;
      if (xx >= w) xx = w - 1;

      double kval = *(kernel + ksize * (dy + r) + (dx + r));
      sum += *(in + yy * w + xx) * kval;
    }
  }

  // clamp result
  if (sum < 0) sum = 0;
  if (sum > 255) sum = 255;
  *(out + y * w + x) = (unsigned char) sum;
  
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

  // allocate images and kernel
  cudaEventCreate (&start);
  cudaEventCreate (&stop);
  cudaEventRecord (start, 0);
  cudaMallocManaged (&kernel, ksize * ksize * sizeof (double));
  e1 = cudaGetLastError ();
  cudaMallocManaged (&image, width * height * sizeof (unsigned char));
  e2 = cudaGetLastError ();
  cudaMallocManaged (&out, width * height * sizeof (unsigned char));
  e3 = cudaGetLastError ();
  cudaDeviceSynchronize ();
  cudaEventRecord (stop, 0);
  cudaEventSynchronize (stop);
  cudaEventElapsedTime (&fcElapsed, start, stop);
  
  if (e1 != cudaSuccess) {
    fprintf (stderr, "Cannot allocate (double *) R.\n");
    exit (1);
  }
  if (e2 != cudaSuccess) {
    fprintf (stderr, "Cannot allocate (char) image");
    exit (1);
  }
  if (e3 != cudaSuccess) {
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

  // build kernel: simple blur
  for (i = 0, kp = kernel; i < ksize * ksize; i++, kp++) {
    *kp = 1.0f / (ksize * ksize);
  }

  device = 0;
  cudaGetDeviceProperties (&properties, device);
  error = cudaGetLastError ();
  if (error != cudaSuccess) {
    fprintf (stderr, "Cannot get CUDA properties.\n");
    exit (1);
  }
  cudaGetDeviceCount (&nDev);
  if (error != cudaSuccess) {
    fprintf (stderr, "Cannot get CUDA number of devices.\n");
    exit (1);
  }

  maxTpB = properties.maxThreadsPerBlock;
  side = (int) sqrt(maxTpB);
  // assume square image
  side = min (side, width);
  blocks = max (ceil ((float) width / side), 1.0);

  dim3 threadsPerBlock (side, side);
  dim3 blocksPerGrid (blocks, blocks);
  
  cudaEventRecord (start, 0);
  filter <<< blocksPerGrid, threadsPerBlock, ksize * ksize * sizeof (double)>>> (
          image, out, kernel, ksize, width, height);
  cudaEventRecord (stop, 0);
  cudaEventSynchronize (stop);
  cudaEventElapsedTime (&gElapsed, start, stop);

  printf ("ksize=%d time_exec_us=%.3f time_total_us=%.3f\n\n", ksize, gElapsed * ms2us, (fcElapsed + gElapsed) * ms2us);

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
