#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdio.h>

    int main (int argc, char *argv[])
    {
      int i;
      int s = socket (PF_INET, SOCK_STREAM, 0);

      for (i=2;;i++)
        {
          struct ifreq ifr;
          struct sockaddr_in *sin = (struct sockaddr_in *) &ifr.ifr_addr;
          char *ip;

          ifr.ifr_ifindex = i;
          if (ioctl (s, SIOCGIFNAME, &ifr) < 0)
            break;

          /* now ifr.ifr_name is set */
          if (ioctl (s, SIOCGIFADDR, &ifr) < 0)
            continue;

          ip = inet_ntoa (sin->sin_addr);
          printf ("%s\n", ip);
        }

      close (s);
      return 0;
    }
