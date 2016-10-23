CC=gcc
CFLAGS=-Wall -g


all: stats.c stat_server.c stats_client.c
	$(CC) -c -fpic stats.c -Wall -Werror -lpthread -lm
	$(CC) -shared -o libstats.so stats.o -lm -lpthread
	$(CC) -o stat_server $(CFLAGS) stat_server.c -lpthread -lm
	$(CC) -o stats_client stats_client.c -Wall -Werror -lstats -L. -lrt

clean:
	$(RM) stats.o
	$(RM) stats_client
	$(RM) stat_server
	$(RM) libstats.so 