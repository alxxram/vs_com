#ifndef VS_COM_H
#define VS_COM_H

#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>

#define BUFFER_SIZE 1024 + 1

// Validation Side Serial Communications Class
class VSCom 
{
    public:
        VSCom() : m_fd(0), m_ttydev("/dev/ttyUSB0"), m_alpha(false) {}
        ~VSCom() { if (m_fd) close(m_fd); }

        int ParseArgs(int c, char **v);
        int Connect();           // Open/configure the serial port
        ssize_t ReadAndPrint();  // Read serial data and print it

    private:
        int m_fd;              // The file descriptor for the serial port
        const char *m_ttydev;  // Path to the serial device
        bool m_alpha;          // Alpha mode indicator
        unsigned char m_iobuffer[BUFFER_SIZE]; // Where data will be read in to

        void AlphaMode(ssize_t nbytes);  // Validate alphabet received
        void PrintData(ssize_t nbytes);  // Print received data
};

#endif
