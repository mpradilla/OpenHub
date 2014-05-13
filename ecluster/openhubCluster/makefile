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
all: openhub_daemon
	$(CC) $(CFLAGS) -o bin/openhub_daemon lib/openhub_daemon.o lib/thpool.o lib/queue.o lib/clustering_cost.o lib/propagation_cost.o

# ***************************
# * Compile openhub daemon     *
# ***************************
openhub_daemon: thpool queue clustering propagation
	$(CC) $(CFLAGS) -c src/openhub_daemon.c -o lib/openhub_daemon.o $(INCLUDES)
	
# ***************************
# * Compile the propagation cost algorithm  *
# ***************************
propagation: 
	$(CC) $(CFLAGS) -c src/propagation_cost.c -o lib/propagation_cost.o $(INCLUDES)
	
# ***************************
# * Compile the clustering cost algorithm  *
# ***************************
clustering: 
	$(CC) $(CFLAGS) -c src/clustering_cost.c -o lib/clustering_cost.o $(INCLUDES)
	
# ***************************
# * Compile queue  *
# ***************************
queue: 
	$(CC) $(CFLAGS) -c src/queue.c -o lib/queue.o $(INCLUDES)
	
# ***************************
# * Compile the Thread pool *
# ***************************
thpool: 
	$(CC) $(CFLAGS) -c src/thpool.c -o lib/thpool.o $(INCLUDES)

# ***************************
# * Clean oveall programm   *
# ***************************
clean:
	rm -f bin/openhub_daemon lib/*.o
	
