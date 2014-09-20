#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <fcntl.h>
#include <errno.h>

#define DEFAULT_TTYDEV "/dev/ttyUSB0"
#define BUFFER_SIZE    1024

static const char *g_ttydev = DEFAULT_TTYDEV;
static char g_iobuffer[BUFFER_SIZE];

// Show_usage
static void show_usage(const char *progname, int exitcode)
{
  fprintf(stderr, "USAGE: %s [<ttydev>]\n", progname);
  exit(exitcode);
}

int main(int argc, char **argv)
{
  struct termios tty;
  ssize_t nbytes;
  int fd;
  int ret;

  // Handle input parameters
  if (argc > 1)
    {
      if (argc > 2)
        {
          fprintf(stderr, "Too many arguments on command line\n");
          show_usage(argv[0], 1);
        }
      g_ttydev = argv[1];
    }

  // Open the USB serial device for blocking read
  do
    {
      printf("Opening USB serial driver\n");
      fd = open(g_ttydev, O_RDWR);
      if (fd < 0)
        {
          printf("ERROR: Failed to open %s: %s\n", g_ttydev, strerror(errno));
          printf("       Assume not connected. Wait and try again.\n");
          printf("       (Control-C to terminate).\n");
          sleep(5);
        }
    } while (fd < 0);
  printf("Successfully opened the serial driver\n");

  // Configure the serial port in raw mode (at least turn off echo)
  ret = tcgetattr(fd, &tty);
  if (ret < 0)
    {
      printf("ERROR: Failed to get termios for %s: %s\n", g_ttydev, strerror(errno));
      close(fd);
      return 1;
    }

  tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON);
  tty.c_oflag &= ~OPOST;
  tty.c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
  tty.c_cflag &= ~(CSIZE|PARENB);
  tty.c_cflag |= CS8;

  ret = tcsetattr(fd, TCSANOW, &tty);
  if (ret < 0)
    {
      printf("ERROR: Failed to set termios for %s: %s\n", g_ttydev, strerror(errno));
      close(fd);
      return 1;
    }

  // Wait for messages forever
  unsigned char prev = 0;
  for (;;)
    {
      nbytes = read(fd, g_iobuffer, BUFFER_SIZE-1);
      if (nbytes < 0)
        {
          printf("ERROR: Failed to read from %s: %s\n", g_ttydev, strerror(errno));
          close(fd);
          return 2;
        }
      else if (nbytes == 0)
        {
          printf("End-of-file encountered\n");
          break;
        }

      g_iobuffer[nbytes] = '\0';
      printf("\r%s", g_iobuffer);
      fflush(stdout);
      
      int i;
      for(i = 0; i < nbytes; i++) {
          if ((g_iobuffer[i] != (prev + 1)) &&
              ((prev == 90) && (g_iobuffer[i] != 65))) {
            printf ("prev:%d  buf:%d\n", prev, g_iobuffer[i]);
          }
          prev = g_iobuffer[i];
      }
    }

  close(fd);
  return 0;
}

