#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

static const char *g_ttydev = "/dev/ttyUSB0";
static char g_iobuffer[BUFFER_SIZE];

// Validation Side Communications Class
class VSCom 
{
    public:
        VSCom();               // Use default serial port
        VSCom(char *dev_path); // Open given device
        int ParseArgs(int c, char **v);
        int Connect();         // Open the serial port

    private:
        int m_fd;               // The file descriptor for the serial port
        char *m_dev_path;       // The path to the port in /dev
        unsigned char m_buffer[BUFFER_SIZE]; // Where data will be read in to
        struct termios m_tty;   // The serial port's configuration structure
        ssize_t m_nbytes;       // How many bytes are read at a time
};

int ParseArgs(int argc, char **argv)
{
    int opt;
    while ((opt = getopt(argc, argv, "d:")) != -1) {
        switch (opt) {
            case 'd':
                g_ttydev = optarg;
            break;
            default:
                fprintf(stderr, "Usage: %s [-d dev_path]\n", argv[0]);
                exit(2); //TODO Return error code instead
        }
    }
    return 0;
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

    // Configure the serial port in raw mode (at least turn off echo)
    ret = tcgetattr(fd, &tty);
    if (ret < 0) {
        printf("ERROR: Failed to get termios for %s: %s\n", g_ttydev, strerror(errno));
        close(fd);
        exit(3);
    }

    tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON);
    tty.c_oflag &= ~OPOST;
    tty.c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
    tty.c_cflag &= ~(CSIZE|PARENB);
    tty.c_cflag |= CS8;

    ret = tcsetattr(fd, TCSANOW, &tty);
    if (ret < 0) {
        printf("ERROR: Failed to set termios for %s: %s\n", g_ttydev, strerror(errno));
        close(fd);
        exit(4);
    }

    return 0;
}

int main(int argc, char **argv)
{
    //struct termios tty;
    ssize_t nbytes;
    int fd;
    //int ret;

    // VSCom vs_com;
    ParseArgs(argc, argv);
    Connect(fd);

    // Wait for messages forever
    unsigned char prev = 0;
    for (;;) {
        nbytes = read(fd, g_iobuffer, BUFFER_SIZE);
        if (nbytes < 0) {
          printf("ERROR: Failed to read from %s: %s\n", g_ttydev, strerror(errno));
          close(fd);
          exit(5);
        } else if (nbytes == 0) {
          printf("End-of-file encountered\n");
          break;
        }

        for (int i = 0; i < nbytes; i++) {
            printf("%c ", g_iobuffer[i]);
            if (g_iobuffer[i] == 'Z') {
                printf("\n");
            }
        }
      
        // Validate the data
        // *HACK!  It is checking vs a hard-coded pattern
        for (int j = 0; j < nbytes; j++) {
            if ((g_iobuffer[j] != (prev + 1)) &&
                ((prev == 90) && (g_iobuffer[j] != 65))) {
                printf("nbytes:%zd j:%d prev:%d  buf:%d\n", nbytes, j, prev, g_iobuffer[j]);
            }
            prev = g_iobuffer[j];
        }
    }

    close(fd);
    return 0;
}

