/** To build:
     gcc vs_com.c -DCONFIG_EXAMPLES_USBSERIAL_INONLY -o vs_com
**/

#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>
#include <fcntl.h>
#include <errno.h>

/****************************************************************************
 * Definitions
 ****************************************************************************/

#if defined(CONFIG_EXAMPLES_USBSERIAL_INONLY) && defined(CONFIG_EXAMPLES_USBSERIAL_OUTONLY)
#  error "Cannot define both CONFIG_EXAMPLES_USBSERIAL_INONLY and _OUTONLY"
#endif
#if defined(CONFIG_EXAMPLES_USBSERIAL_ONLYSMALL) && defined(CONFIG_EXAMPLES_USBSERIAL_ONLYBIG)
#  error "Cannot define both CONFIG_EXAMPLES_USBSERIAL_ONLYSMALL and _ONLYBIG"
#endif

#if !defined(CONFIG_EXAMPLES_USBSERIAL_ONLYBIG) && !defined(CONFIG_EXAMPLES_USBSERIAL_ONLYSMALL)
#  ifndef CONFIG_EXAMPLES_USBSERIAL_INONLY
#    define COUNTER_NEEDED 1
#  endif
#endif

#ifdef CONFIG_CDCACM
#  define DEFAULT_TTYDEV "/dev/ttyACM0"
#else
#  define DEFAULT_TTYDEV "/dev/ttyUSB0"
#endif
#define BUFFER_SIZE    1024

/****************************************************************************
 * Private Data
 ****************************************************************************/

static const char *g_ttydev = DEFAULT_TTYDEV;

#ifndef CONFIG_EXAMPLES_USBSERIAL_ONLYBIG
static const char g_shortmsg[] = "Sure... You betcha!!\n";
#endif

#ifndef CONFIG_EXAMPLES_USBSERIAL_ONLYSMALL
static const char g_longmsg[] =
  "I am proud to come to this city as the guest of your distinguished Mayor, "
  "President John F. Kennedy - June 26, 1963\n";
#endif

static char g_iobuffer[BUFFER_SIZE];

/****************************************************************************
 * show_usage
 ****************************************************************************/
static void show_usage(const char *progname, int exitcode)
{
  fprintf(stderr, "USAGE: %s [<ttydev>]\n", progname);
  exit(exitcode);
}

int main(int argc, char **argv, char **envp)
{
  struct termios tty;
#ifndef CONFIG_EXAMPLES_USBSERIAL_OUTONLY
  ssize_t nbytes;
#endif
#ifdef COUNTER_NEEDED
  int count = 0;
#endif
  int fd;
  int ret;

  /* Handle input parameters */

  if (argc > 1)
    {
      if (argc > 2)
        {
          fprintf(stderr, "Too many arguments on command line\n");
          show_usage(argv[0], 1);
        }
      g_ttydev = argv[1];
    }

  /* Open the USB serial device for blocking read/write */

  do
    {
      printf("main: Opening USB serial driver\n");
      fd = open(g_ttydev, O_RDWR);
      if (fd < 0)
        {
          printf("main: ERROR: Failed to open %s: %s\n", g_ttydev, strerror(errno));
          printf("main:        Assume not connected. Wait and try again.\n");
          printf("main:        (Control-C to terminate).\n");
          sleep(5);
        }
    }
  while (fd < 0);
  printf("main: Successfully opened the serial driver\n");

  /* Configure the serial port in raw mode (at least turn off echo) */

  ret = tcgetattr(fd, &tty);
  if (ret < 0)
    {
      printf("main: ERROR: Failed to get termios for %s: %s\n", g_ttydev, strerror(errno));
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
      printf("main: ERROR: Failed to set termios for %s: %s\n", g_ttydev, strerror(errno));
      close(fd);
      return 1;
    }

  /* Wait for and/or send messages -- forever */

  unsigned char prev = 0;
  for (;;)
    {
      nbytes = read(fd, g_iobuffer, BUFFER_SIZE-1);
      if (nbytes < 0)
        {
          printf("main: ERROR: Failed to read from %s: %s\n", g_ttydev, strerror(errno));
          close(fd);
          return 2;
        }
      else if (nbytes == 0)
        {
          printf("main: End-of-file encountered\n");
          break;
        }

      g_iobuffer[nbytes] = '\0';
      //printf("main: Received %ld bytes:", nbytes);
      //printf("      \"%s\"\n", g_iobuffer);
      printf("\r%s", g_iobuffer);
      fflush(stdout);
      //if (g_iobuffer[nbytes-1] == 90) {
      //    printf("\n");
      //}
      
      int i;
      //printf("\n");
      for(i = 0; i < nbytes; i++) {
          //printf("%x", g_iobuffer[i]);
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

