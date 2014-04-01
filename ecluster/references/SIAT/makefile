# define the C compiler to use
CC = mpicc

# define any compile-time flags
CFLAGS = -Wall  -g 
CUDPFLAGS = -g 
#-Wextra -pedantic

# define any directories containing header files other than /usr/include
#
INCLUDES = -Iinclude

# ***************************
# * Compile all programm    *
# ***************************
all: siat_daemon
	$(CC) $(CFLAGS) -o bin/siat_daemon lib/siat_daemon.o lib/thpool.o lib/queue.o lib/udpserver.o lib/list.o

# ***************************
# * Compile siat daemon     *
# ***************************
siat_daemon: thpool queue udp list
	$(CC) $(CFLAGS) -c source/siat_daemon.c -o lib/siat_daemon.o $(INCLUDES)
	
# ***************************
# * Compile the UDP server  *
# ***************************
udp: queue
	$(CC) $(CUDPFLAGS) -c source/udpserver.c -o lib/udpserver.o $(INCLUDES)
	
# ***************************
# * Compile queue  *
# ***************************
list: 
	$(CC) $(CFLAGS) -c source/list.c -o lib/list.o $(INCLUDES)
	
# ***************************
# * Compile queue  *
# ***************************
queue: 
	$(CC) $(CFLAGS) -c source/queue.c -o lib/queue.o $(INCLUDES)
	
# ***************************
# * Compile the Thread pool *
# ***************************
thpool: 
	$(CC) $(CFLAGS) -c source/thpool.c -o lib/thpool.o $(INCLUDES)

# ***************************
# * Clean oveall programm   *
# ***************************
clean:
	rm -f bin/siat_daemon lib/*.o
	
