#define SRV_IP "157.253.203.27"
#include "arpa/inet.h"
#include "netinet/in.h"
#include "stdio.h"
#include "sys/types.h"
#include "sys/socket.h"
#include "unistd.h"
#include "string.h"
#include "stdlib.h"

#define BUFLEN 1024
#define NPACK 10
#define PORT 5000

void error(char *s)
{
    perror(s);
    exit(1);
}

  int main(void)
  {
    struct sockaddr_in si_other;
  int s, i, slen=sizeof(si_other);
    char buf[BUFLEN];

    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
      error("socket");
    memset((char *) &si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(PORT);
    if (inet_aton(SRV_IP, &si_other.sin_addr)==0)
      error("inet_aton:");

   // for (i=0; i<NPACK; i++) {
   i = 0;
	while (i < 100000){
		printf("Sending packet %d\n", i);
		sprintf(buf,":begin:%d&%d&%f&%f&%f&%f&%f&%f:end:",i,i,i+1,i+100.01,i+1030.01,i+10400.01,i+106000.01+1,8.00);
		//sprintf(buf, "This is packet %d\n", i);
		if (sendto(s, buf, BUFLEN, 0, (struct sockaddr *)&si_other, slen)==-1)
			error("sendto()");
		//sleep(1);
		usleep(5000);
		++i;
    }

    close(s);
    return 0;
  }