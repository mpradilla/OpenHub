#ifndef SOCKETSERVER_H
#define SOCKETSERVER_H

#include "queue.h"
#include "list.h"

void error(char *s);

void sigio_handler(int sig);

int enable_asynch(int sock, dataqueue *dataqueue_p);

void socketpua(dataqueue *dataqueue_p);

void test(void);

void socketclientC3(int sensor, long counter, double time, double value);

#endif