#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

long random_at_most(long max) {
    unsigned long

    num_bins = (unsigned long) max + 1,
    num_rand = (unsigned long) RAND_MAX + 1,
    bin_size = num_rand / num_bins,
    defect   = num_rand % bin_size;

    long x;
    while (num_rand - defect <= (unsigned long)(x = random()));

    return x/bin_size;
}

int main()
{
    FILE *fp;

    short int sdata;

    for (sdata = 0; sdata < 5000; sdata++) { 
        fp = fopen("mag_z.txt", "w+");

        long data = random_at_most(7000);
        fwrite(&data, 4, 1, fp);
        fflush(fp);
        fclose(fp);
        usleep(500);
    }
}
