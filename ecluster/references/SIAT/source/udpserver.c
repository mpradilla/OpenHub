// TOMADO DE
// http://www.lindusembedded.com/blog/2008/11/06/simple-udp-client-server-with-asynchronous-io/

#define _GNU_SOURCE
#include "arpa/inet.h"
#include "netinet/in.h"
#include "stdio.h"
#include "sys/types.h"
#include "sys/socket.h"
#include "unistd.h"
#include "string.h"
#include "stdlib.h"
#include "signal.h"
#include "unistd.h"
#include "fcntl.h"

#include "udpserver.h"
#include "queue.h"
#include "list.h"

#define BUFLEN 1024
#define PORT 5000
//#define SRV_C3_IP "157.253.203.12"
#define SRV_C3_IP "157.253.216.28"
#define PORT_C3 6000

//#define ASYNC

int sock;

void error(char *s){
    perror(s);
    exit(1);
}

void sigio_handler(int sig){
   char buffer[BUFLEN]="";
   static int count = 0;
   struct sockaddr_in si_other;
   int slen=sizeof(si_other);
    data_item *newdata_p;

   if (recvfrom(sock, buffer, BUFLEN, 0, (struct sockaddr *)&si_other, &slen)==-1)
       error("recvfrom()");
   printf("Received packet from %s:%d\nData: %s\n\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port), buffer);
}

int enable_asynch(int sock, dataqueue *dataqueue_p){
  int stat = -1;
  int flags;
  struct sigaction sa;

  flags = fcntl(sock, F_GETFL);
  fcntl(sock, F_SETFL, flags | O_ASYNC); 

  sa.sa_flags = 0;
  sa.sa_handler = sigio_handler;
  sigemptyset(&sa.sa_mask);

  if (sigaction(SIGIO, &sa, NULL))
    error("Error:");

  if (fcntl(sock, F_SETOWN, getpid()) < 0)
    error("Error:");

  if (fcntl(sock, F_SETSIG, SIGIO) < 0)
    error("Error:");
  return 0;
}

void socketpua(dataqueue *dataqueue_p){
	
   struct sockaddr_in si_me, si_other;
   int	slen;
   char	buf[BUFLEN];

   data_item *newdata_p;
   slen = sizeof(si_other);

   if ((sock=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
      error("socket");
	
	memset((char *) &si_me, 0, sizeof(si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(PORT);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(sock, (struct sockaddr *)&si_me, sizeof(si_me))==-1)
        error("bind");
		
#ifdef ASYNC
	enable_asynch(sock,dataqueue_p);
	while(1) { pause(); }

#else 
	while(1){
		//bytes_read = recvfrom(sock,recv_data,1024,0,(struct sockaddr *)&client_addr, &addr_len);
		if (recvfrom(sock, buf, BUFLEN, 0, (struct sockaddr *)&si_other, &slen) == -1)
			error("recvfrom()");
	   /* añadido */
	   //recv_data[bytes_read] = '\0';
		newdata_p = (data_item *)malloc(sizeof(data_item));
		newdata_p->next = NULL;
		newdata_p->prev = NULL;
		strcpy(newdata_p->data,buf);
		dataqueue_add(dataqueue_p, newdata_p);
    }
    close(sock);
#endif
}

void socketclientC3(int sensor, long counter, double time, double value){
	
    struct sockaddr_in si_other;
	int s, slen=sizeof(si_other);
    char buf[BUFLEN];
	
    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
      error("socket");

	memset((char *) &si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(PORT_C3);

    if (inet_aton(SRV_C3_IP, &si_other.sin_addr)==0)
      error("inet_aton:");
	//ACA SE COLOCA LA TRAMA A ENVÍAR
	
	sprintf(buf,":begin:%d&%zu&%f&%f:end:",sensor, counter, time, value);
	printf("buf %s\n",buf);
	if (sendto(s, buf, BUFLEN, 0, (struct sockaddr *)&si_other, slen)==-1)
		error("sendto()");
	//sleep(1);
	usleep(500);
    close(s);
  }