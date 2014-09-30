#include "vs_com.h"

const char *g_ttydev = "/dev/ttyUSB0";
unsigned char g_iobuffer[BUFFER_SIZE];
bool alpha = false;
int fd;

int main(int argc, char **argv)
{
    ssize_t nbytes;
    int ret;

    VSCom vs_com;

    // Parse command line arguments
    if ((ret = vs_com.ParseArgs(argc, argv)) < 0) {
        exit(ret);
    }

    // Open and configure serial port
    if ((ret = vs_com.Connect()) < 0) {
        exit(ret);
    }

    // Wait for messages forever
    while (1) {
        nbytes = vs_com.ReadAndPrint();
        if (nbytes < 0) {
            exit(nbytes);
        }
        
        usleep(10);
    }

    return 0;
}

