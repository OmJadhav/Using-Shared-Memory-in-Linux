CC=gcc
CFLAGS=-Wall -g


all: stats.c stats_server.c stats_client.c
	$(CC) -c -fpic stats.c -Wall -Werror -lpthread -lm
	$(CC) -shared -o libstats.so stats.o -lm -lpthread
	$(CC) -o stats_server $(CFLAGS) stats_server.c -lpthread -lm
	$(CC) -o stats_client stats_client.c -Wall -Werror -lstats -L. -lrt

clean:
	$(RM) stats.o
	$(RM) stats_client
	$(RM) stats_server
	$(RM) libstats.so 
