#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/shm.h>
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "stats.h"


// global key
int key = 0;

void
usage(char *prog) {
  fprintf(stderr,
    "usage: %s -k key -p priority -s sleeptime_ns -c cputime_ns -v \n", prog);
  exit(1);
}


// signal handler
void
signal_cleanup(int signum) {
  if (signum == SIGINT) {
    int ret = stats_unlink(key);
    exit(ret);
  }
}


int
main(int argc, char *argv[]) {
  // arguments
  int priority = 0;
  int sleeptime_ns = 10000;
  int cputime_ns = 10000;

  // signal structure
  struct sigaction action;
  // input params
  int c, key_found = 0;
  opterr = 0;
  if (argc > 9 || argc < 3) {
    usage(argv[0]);
  }
  while ((c = getopt(argc, argv, "k:p:s:c:")) != -1) {
    switch (c) {
    case 'k':
      key = atoi(optarg);
      // TO DO: add asserts here later
      key_found = 1;
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
  if (!key_found) {
    usage(argv[0]);
  }
  // print the argv
  printf("key %d priority %d sleeptime_ns %d cputime_ns %d\n",
    key, priority, sleeptime_ns, cputime_ns);

  // handle signal
  action.sa_handler = signal_cleanup;
  // sigemptyset(&action.sa_mask);
  sigaction(SIGINT, &action, NULL);

  // if the priority is less than 0 exit
  if (priority < 0) {
    fprintf(stderr, "process priority is less than 0");
    exit(1);
  }

/*  while(1) {
    printf("in while looop\n");
  }
*/
  stats_t pinfo;
  pinfo.pid = getpid();
  // printf("pid : %d-----------\n", pinfo.pid);
  strcpy(pinfo.program_name, "");
  strncat(pinfo.program_name, argv[0], 15);
  pinfo.counter = 0;
  // set rhe priority of the process
  setpriority(PRIO_PROCESS, 0, priority);
  pinfo.priority = getpriority(PRIO_PROCESS, 0);
  pinfo.cpu_secs = 0;

  stats_t *shm_ptr;
  shm_ptr = stats_init(key);
  /*while (1) {
    shm_ptr = stats_init(key);
    if (shm_ptr != NULL) {
      break;
    }
  }*/
  if (shm_ptr == NULL) {
      exit(-1);
  }
  *shm_ptr = pinfo;

  struct timespec base_time, elapsed_time, last_ran_time, req_time;

  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &last_ran_time);

  req_time.tv_sec = sleeptime_ns / 999999999;
  req_time.tv_nsec = sleeptime_ns - (sleeptime_ns / 999999999);

  while (1) {
    // first, client should sleep
    nanosleep(&req_time, NULL);

    // next, client should busy-wait
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &base_time);

    while (1) {
      clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &elapsed_time);
      if (base_time.tv_sec == elapsed_time.tv_sec) {
        if (elapsed_time.tv_nsec - base_time.tv_nsec >= cputime_ns) {
          break;
        }
      } else {
        long diff = (elapsed_time.tv_sec - base_time.tv_sec) * 1000000000 +
          (elapsed_time.tv_nsec - base_time.tv_nsec);
        if (diff >= cputime_ns) {
          break;
        }
      }
    }  //  end of while busy wait

    shm_ptr->counter++;

    time_t old_sec = last_ran_time.tv_sec;
    long old_nsec = last_ran_time.tv_nsec;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &last_ran_time);
    double diff_last_ran = ((last_ran_time.tv_sec - old_sec) * 1000000000
      + (last_ran_time.tv_nsec - old_nsec)) / (double) 1000000000;
    shm_ptr->cpu_secs += diff_last_ran;
  }

  return 0;
}
