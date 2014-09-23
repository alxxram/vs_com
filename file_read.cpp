#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

int main()
{
    FILE *fp;

    for (int i = 0; i < 1000; i++) {
        fp = fopen("mag_z.txt", "r");

        char *in_data[2];
        fread(in_data, 2, 1, fp);
        fclose(fp);

        int16_t print_data = 0;
        memcpy(&print_data, in_data, 2);
        printf("%hd\n", print_data);
        sleep(1);
    }
/*
    char dots[100];

    for (int i = 0; i < 100; i++) {
        dots[i] = '.';
    }
    dots[sizeof(dots) - 1] = 0;

    for (int j; j < 50000; j++) {
        printf("\r%s", dots); 
        fflush(stdout);
        if (j % 2) {
            snprintf(&dots[70], 30, "%s", "                              ");
        } else {
            snprintf(&dots[70], 30, "%s", "..............................");
        }
        usleep(500);
    }
*/
}
