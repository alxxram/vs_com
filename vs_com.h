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

extern const char *g_ttydev;
extern unsigned char g_iobuffer[BUFFER_SIZE];
extern bool alpha;
extern int fd;

int ParseArgs(int argc, char **argv);
int Connect(int &fd);
void AlphaMode(ssize_t nbytes);
void PrintData(ssize_t nbytes);
ssize_t ReadAndPrint();

// Validation Side Serial Communications Class
class VSCom 
{
    public:
        VSCom();
        int ParseArgs(int c, char **v);
        int Connect();         // Open/configure the serial port
        void AlphaMode();      // Validate alphabet received

    private:
        int m_fd;               // The file descriptor for the serial port
        char *m_dev_path;       // The path to the port in /dev
        unsigned char m_buffer[BUFFER_SIZE]; // Where data will be read in to
        struct termios m_tty;   // The serial port's configuration structure
        ssize_t m_nbytes;       // How many bytes are read at a time
};

#endif
