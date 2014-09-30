#include "vs_com.h"

int VSCom::ParseArgs(int argc, char **argv)
{
    int opt;
    while ((opt = getopt(argc, argv, "ad:")) != -1) {
        switch (opt) {
            case 'd':
                m_ttydev = optarg;
            break;
            case 'a':
                m_alpha = true;
            break;
            default:
                fprintf(stderr, "Usage: %s [-d dev_path]\n", argv[0]);
                return(-1);
        }
    }

    return(0);
}

int VSCom::Connect()
{
    int ret;
    struct termios tty;

    // Open the USB serial device for blocking read
    do {
        printf("Opening USB serial driver\n");
        m_fd = open(m_ttydev, O_RDWR);
        if (m_fd < 0) {
            printf("ERROR: Failed to open %s: %s\n", m_ttydev, strerror(errno));
            printf("       Assume not connected. Wait and try again.\n");
            printf("       (Control-C to terminate).\n");
            sleep(5);
        }
    } while (m_fd < 0);
    printf("Successfully opened the serial driver\n");

    ret = tcgetattr(m_fd, &tty);
    if (ret < 0) {
        fprintf(stderr, "ERROR: Failed to get termios for %s: %s\n", m_ttydev, strerror(errno));
        close(m_fd);
        return(-1);
    }

    tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON);
    tty.c_oflag &= ~OPOST;
    tty.c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
    tty.c_cflag &= ~(CSIZE|PARENB);
    tty.c_cflag |= CS8;

    ret = tcsetattr(m_fd, TCSANOW, &tty);
    if (ret < 0) {
        fprintf(stderr, "ERROR: Failed to set termios for %s: %s\n", m_ttydev, strerror(errno));
        close(m_fd);
        return(-1);
    }

    return(0);
}

void VSCom::AlphaMode(ssize_t nbytes)
{
    for (int i = 0; i < nbytes; i++) {
        printf("%c ", m_iobuffer[i]);
        if (m_iobuffer[i] == 'Z') {
            printf("\n");
        }
    }

    // Validate the data
    unsigned char prev = 0;
    for (int j = 0; j < nbytes; j++) {
        if ((m_iobuffer[j] != (prev + 1)) &&
            ((prev == 90) && (m_iobuffer[j] != 65))) {
            printf("nbytes:%zd j:%d prev:%d  buf:%d\n", nbytes, j, prev, m_iobuffer[j]);
        }
        prev = m_iobuffer[j];
    }
}

void VSCom::PrintData(ssize_t nbytes)
{
    // Print the raw data
    for (int i = 0; i < nbytes; i++) {
        printf("0x%x ", m_iobuffer[i]);
    }

    // Then print the data as a string
    assert(nbytes < BUFFER_SIZE);
    m_iobuffer[nbytes] = 0;
    printf("\n%s\n\n", m_iobuffer);
}

ssize_t VSCom::ReadAndPrint()
{
    // Read up to BUFFER_SIZE bytes
    ssize_t nbytes = read(m_fd, m_iobuffer, BUFFER_SIZE - 1);

    if (nbytes < 0) {
        fprintf(stderr, "ERROR: Failed to read from %s: %s\n", m_ttydev, strerror(errno));
        close(m_fd);
        return(nbytes);
    } else if (nbytes == 0) {
        fprintf(stderr, "WARNING: End-of-file encountered\n");
        return(nbytes);
    }

    if (m_alpha) {
        AlphaMode(nbytes);
    } else {
        PrintData(nbytes);
    }

    return(nbytes);
}
