#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void write_pgm(const char *filename, int w, int h) {
    FILE *f = fopen(filename, "wb");
    if (!f) {
        fprintf(stderr, "fopen error\n");
        exit(1);
    }

    fprintf(f, "P5\n%d %d\n255\n", w, h);  // P5 binary PGM header

    unsigned char *data = malloc(w * h);
    if (!data) {
        fprintf(stderr, "malloc error");
        exit(1);
    }

    for (int i = 0; i < w * h; i++) {
        data[i] = rand() % 256;  // random pixel [0..255]
    }

    fwrite(data, 1, w * h, f);
    fclose(f);
    free(data);
}

int main(void) {
    srand(time(NULL));  // seed random generator

    int sizes[] = {32, 64, 128, 256, 512, 1024, 2048, 4096, 8192};
    int n = 9;

    for (int i = 0; i < n; i++) {
        char filename[64];
        sprintf(filename, "input_%d.pgm", sizes[i]);
        write_pgm(filename, sizes[i], sizes[i]);
        printf("Generated %s\n", filename);
    }

    return 0;
}
