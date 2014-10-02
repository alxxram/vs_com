#include "vs_com.h"

/// \brief Will parse the command line arguments and set member variables
///
/// \param argc The argc from main
/// \param argv The argv from main
/// \return 0 on success, -1 on failure
///
/// This function uses Boost to parse the command line arguments.
/// The parsing uses a try/catch block to handle invalid parameter
/// errors.
///
/// The command line arguments for this module are:
///     --help (-h)     Prints the usage message
///     --alpha (-a)    Enables Alpha mode which validates that the alphabet is received
///     --path (-p)     Path to the serial device i.e. /dev/ttyUSB0
///     --do_not_print (-D)  Do not print received data to the screen
///
int VSCom::ParseArgs(int argc, char **argv)
{
    namespace po = boost::program_options;

    po::options_description desc("Usage");
    desc.add_options()
        ("help,h", "Display this help message")
        ("alpha,a", "Enable Alpha mode")
        ("path,p", po::value<string>(), "Path to serial device")
        ("do_not_print,D", "Do not print received data to screen");

    po::variables_map vm;
    try {
        po::store(po::parse_command_line(argc, argv, desc), vm); // This can throw

        if (vm.count("alpha")) {
           m_alpha = true;
        }

        if (vm.count("path")) {
            m_ttydev = vm["path"].as<string>().c_str();
        }

        if (vm.count("do_not_print")) {
            m_print = false;
        }

        if (vm.count("help")) {
            cout << desc << endl;
            exit(0);
        }

        po::notify(vm);
    }
    catch(po::error &e) {
        cerr << "ERROR: " << e.what() << endl;
        cerr << desc << endl;
        return(-1);
    }

    return(0);
}

/// \brief Opens and configures a serial connection
///
/// \return 0 on success, -1 on failure
///
/// Will open and configure the serial port at the given path.  The port is configured for
/// blocking reads.
/// TODO B57600 baud rate is hardcoded.  Allow user to specify.
///
int VSCom::Connect()
{
    int ret;
    struct termios tty;

    // Open the serial device for blocking read
    do {
        cout << "Opening serial device" << endl;
        m_fd = open(m_ttydev, O_RDWR);
        if (m_fd < 0) {
            cout << "ERROR: Failed to open " << m_ttydev << ": " << strerror(errno) << endl;
            cout << "       Assume not connected. Wait and try again." << endl;
            cout << "       (Control-C to terminate)." << endl;
            sleep(5);
        }
    } while (m_fd < 0);
    cout << "Successfully opened the serial driver" << endl;

    ret = tcgetattr(m_fd, &tty);
    if (ret < 0) {
        cerr << "ERROR: Failed to get termios for " << m_ttydev << ": " <<  strerror(errno) << endl;
        close(m_fd);
        return(-1);
    }

    // TODO  Allow user to specify baud
    tty.c_cflag = B57600 | CRTSCTS | CS8 | CLOCAL | CREAD;
    tty.c_iflag = IGNPAR;
    tty.c_oflag = 0;
    tty.c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);

    ret = tcsetattr(m_fd, TCSANOW, &tty);
    if (ret < 0) {
        cerr << "ERROR: Failed to set termios for " << m_ttydev << ": " <<  strerror(errno) << endl;
        close(m_fd);
        return(-1);
    }

    return(0);
}

/// \brief Will validate that the alphabet in caps was received
///
/// \param nbytes The number of bytes received by the read() call
/// \return 0 on success, -1 on failure
///
/// This can be used to validate the serial connection.  It will assume that the alphabet, in all
/// caps, will be sent.  It will print to the screen whatever it receives.  It will print to
/// stderr if there is a failure.
///
int VSCom::AlphaMode(ssize_t nbytes)
{
    // Print the letters to the screen
    for (int i = 0; i < nbytes; i++) {
        cout << m_iobuffer[i];
        if (m_iobuffer[i] == 'Z') {
            cout << endl;
        }
    }

    // Validate the data sequence
    unsigned char prev = 0;
    for (int j = 0; j < nbytes; j++) {
        if ((m_iobuffer[j] != (prev + 1)) &&
            ((prev == 90) && (m_iobuffer[j] != 65))) {
            cerr << "ERROR: Validation of the alpha sequence failed." << endl;
            cerr << "nbytes:" << nbytes << " j:" << j;
            cerr << " prev:" << prev << " buf:" <<  m_iobuffer[j] << endl;
            return(-1);
        }
        prev = m_iobuffer[j];
    }

    return(0);
}

/// \brief Prints received data to screen
///
/// \param nbytes The number of bytes received by the read() call
///
/// For every read() call, it prints the bytes to the screen.  First, it prints the raw data as
/// a hex value.  Then, on the next line it prints the same data as a string.
///
void VSCom::PrintData(ssize_t nbytes)
{
    // Print the raw data
    for (int i = 0; i < nbytes; i++) {
        cout << "0x" << hex << (unsigned int)m_iobuffer[i] << " ";
    }

    // Then print the data as a string
    assert(nbytes < BUFFER_SIZE);
    m_iobuffer[nbytes] = 0;
    cout << endl << m_iobuffer << endl << endl;
}

/// \brief Reads data from the serial port and returns the number of bytes read
///
/// \return The number of bytes read or negative value on error
///
/// Calls the read() function to read from the serial port and stores it in.
/// Will either print and validate the alphabet if alpha mode is on, or it will just print the
/// raw data to the screen if "print" is enabled.
///
ssize_t VSCom::Read()
{
    // Read up to BUFFER_SIZE-1 bytes
    ssize_t nbytes = read(m_fd, m_iobuffer, BUFFER_SIZE - 1);

    if (nbytes < 0) {
        cerr << "ERROR: Failed to read from " << m_ttydev << ": " << strerror(errno);
        close(m_fd);
        return(nbytes);
    } else if (nbytes == 0) {
        cerr << "WARNING: End-of-file encountered" << endl;
        return(nbytes);
    }

    if (m_alpha) {
        AlphaMode(nbytes);
    } else if (m_print) {
        PrintData(nbytes);
    }

    return(nbytes);
}
