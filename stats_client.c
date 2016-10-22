#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/shm.h>
#include "stats.h"

void
usage(char *prog) 
{
  fprintf(stderr, "usage: %s <-k key> <-p priority> <-s sleeptime_ns> <-c cputime_ns> <-v> \n", prog);
  exit(1);
}


// signal handler 
void
signal_cleanup(int signum) {
  if (signum == SIGINT) {
    printf("cleaning up \n");
    exit(0);
  }
}


int
main(int argc, char *argv[])
{
  // arguments
  int key = 0;
  int priority;
  int sleeptime_ns;
  int cputime_ns;

  // signal structure 
  struct sigaction action;
  // input params
  int c;
  opterr = 0;
  if(argc != 9) {
    usage(argv[0]);
  }
  while ((c = getopt(argc, argv, "k:p:s:c:")) != -1) {
    switch (c) {
    case 'k':
      key = atoi(optarg);
      // TO DO: add asserts here later
      break;
    case 'p':
      priority = atoi(optarg);
      break;
    case 's':
      sleeptime_ns = atoi(optarg);
      break;
    case 'c':
      cputime_ns = atoi(optarg);
      break;
    default:
      usage(argv[0]);
    }
  }

  // print the argv
  printf("key %d priority %d sleeptime_ns %d cputime_ns %d\n", key, priority, sleeptime_ns, cputime_ns);

  // handle signal
  action.sa_handler = signal_cleanup;
  // sigemptyset(&action.sa_mask);
  sigaction(SIGINT, &action, NULL);
  

/*  while(1) {
    printf("in while looop\n");
  }
*/
  stats_t pinfo;
  pinfo.pid = getpid();
  pinfo.counter = 0;
  pinfo.priority = 0;
  pinfo.cpu_secs = 0;

  int page_size = getpagesize();
  int mem_id;
   if ((mem_id = shmget(key, page_size, 0660)) == -1) {
        perror("shmget");
        exit(1);
    }
  stats_t *shm_ptr;
  shm_ptr = shmat(mem_id, (void *)0, 0);

  *shm_ptr = pinfo;




  return 0;
}
