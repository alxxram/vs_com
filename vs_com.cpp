#include "vs_com.h"

int ParseArgs(int argc, char **argv)
{
    int opt;
    while ((opt = getopt(argc, argv, "ad:")) != -1) {
        switch (opt) {
            case 'd':
                g_ttydev = optarg;
            break;
            case 'a':
                alpha = true;
            break;
            default:
                fprintf(stderr, "Usage: %s [-d dev_path]\n", argv[0]);
                return(-1);
        }
    }

    return(0);
}

int Connect(int &fd)
{
    int ret;
    struct termios tty;

    // Open the USB serial device for blocking read
    do {
        printf("Opening USB serial driver\n");
        fd = open(g_ttydev, O_RDWR);
        if (fd < 0) {
            printf("ERROR: Failed to open %s: %s\n", g_ttydev, strerror(errno));
            printf("       Assume not connected. Wait and try again.\n");
            printf("       (Control-C to terminate).\n");
            sleep(5);
        }
    } while (fd < 0);
    printf("Successfully opened the serial driver\n");

    ret = tcgetattr(fd, &tty);
    if (ret < 0) {
        fprintf(stderr, "ERROR: Failed to get termios for %s: %s\n", g_ttydev, strerror(errno));
        close(fd);
        return(-1);
    }

    tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON);
    tty.c_oflag &= ~OPOST;
    tty.c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
    tty.c_cflag &= ~(CSIZE|PARENB);
    tty.c_cflag |= CS8;

    ret = tcsetattr(fd, TCSANOW, &tty);
    if (ret < 0) {
        fprintf(stderr, "ERROR: Failed to set termios for %s: %s\n", g_ttydev, strerror(errno));
        close(fd);
        return(-1);
    }

    return(0);
}

void AlphaMode(ssize_t nbytes)
{
    for (int i = 0; i < nbytes; i++) {
        printf("%c ", g_iobuffer[i]);
        if (g_iobuffer[i] == 'Z') {
            printf("\n");
        }
    }

    // Validate the data
    unsigned char prev = 0;
    for (int j = 0; j < nbytes; j++) {
        if ((g_iobuffer[j] != (prev + 1)) &&
            ((prev == 90) && (g_iobuffer[j] != 65))) {
            printf("nbytes:%zd j:%d prev:%d  buf:%d\n", nbytes, j, prev, g_iobuffer[j]);
        }
        prev = g_iobuffer[j];
    }
}

void PrintData(ssize_t nbytes)
{
    // Print the raw data
    for (int i = 0; i < nbytes; i++) {
        printf("%x ", g_iobuffer[i]);
    }

    // Then print the data as a string
    assert(nbytes < BUFFER_SIZE);
    g_iobuffer[nbytes] = 0;
    printf("\n%s\n\n", g_iobuffer);
}

ssize_t ReadAndPrint()
{
    // Read up to BUFFER_SIZE bytes
    ssize_t nbytes = read(fd, g_iobuffer, BUFFER_SIZE - 1);

    if (nbytes < 0) {
        fprintf(stderr, "ERROR: Failed to read from %s: %s\n", g_ttydev, strerror(errno));
        close(fd);
        return(nbytes);
    } else if (nbytes == 0) {
        fprintf(stderr, "End-of-file encountered\n");
        return(nbytes);
    }

    if (alpha) {
        AlphaMode(nbytes);
    } else {
        PrintData(nbytes);
    }

    return(nbytes);
}

int main(int argc, char **argv)
{
    ssize_t nbytes;
    int ret;

    // VSCom vs_com;

    // Parse command line arguments
    if ((ret = ParseArgs(argc, argv)) < 0) {
        exit(ret);
    }

    // Open and configure serial port
    if ((ret = Connect(fd)) < 0) {
        exit(ret);
    }

    // Wait for messages forever
    for (;;) {
        nbytes = ReadAndPrint();
        if (nbytes < 0) {
            exit(nbytes);
        }
        
        usleep(1);
    }

    close(fd);
    return 0;
}

