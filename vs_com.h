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

#include <boost/program_options.hpp>
#include <string>

#define BUFFER_SIZE 1024 + 1

using namespace std;

/// \brief Validation side serial communications class
///
/// This class can be used to open a serial port which will only receive data.
/// By default it will print the data received to the screen.
///
/// There is also an "Alpha" mode that expects the alphabet (in caps) to be
/// continually come in the I/O stream.
/// In Alpha mode, the stream will be validated.
///
class VSCom 
{
    public:
        /// \brief Constructor initializes some member variables
        ///
        VSCom() : m_fd(0), m_ttydev("/dev/ttyUSB0"), m_alpha(false), m_print(true) { }

        /// \brief Desctrcutor closes the serial file descriptor
        ///
        ~VSCom() { if (m_fd) close(m_fd); }

        /// \brief Overrides default settings with user arguments
        ///
        int ParseArgs(int argc, char **argv);

        /// \brief Opens and configures a serial port
        ///
        int Connect();

        /// \brief Reads data from the serial port and optionally prints it
        ///
        ssize_t Read();

    private:
        int m_fd;             /// The file descriptor for the serial port
        const char *m_ttydev; /// Path to the serial device
        bool m_alpha;         /// Alpha mode if true
        bool m_print;         /// Print data to screen if true
        unsigned char m_iobuffer[BUFFER_SIZE]; /// Where data will be read in to

        /// \brief Validate that the data received is the alphabet (in caps)
        ///
        int AlphaMode(ssize_t nbytes);

        /// \brief Will print the data to the screen in hex and as a string
        ///
        void PrintData(ssize_t nbytes);
};

#endif
