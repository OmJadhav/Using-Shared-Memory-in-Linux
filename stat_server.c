#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "stats.h"

void
usage(char *prog) 
{
  fprintf(stderr, "usage: %s <-k key> \n", prog);
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
  if(argc != 3) {
    usage(argv[0]);
  }
  while ((c = getopt(argc, argv, "k:")) != -1) {
    switch (c) {
    case 'k':
      key = atoi(optarg);
      // TO DO: add asserts here later
      break;
    default:
      usage(argv[0]);
    }
  }

  // print the argv
  printf("key %d \n", key);

  // handle signal
  action.sa_handler = signal_cleanup;
  // sigemptyset(&action.sa_mask);
  sigaction(SIGINT, &action, NULL);

  // Shared mem create 
  int page_size = getpagesize();
  printf("page size %d\n",page_size);
  printf("before shmget call\n");
  int mem_id;
   if ((mem_id = shmget(key, page_size, 0660 | IPC_CREAT)) == -1) {
        perror("shmget");
        exit(1);
    }

  // shmget(key, page_size, IPC_CREAT | 0x666);
  printf("after shmget call\n");
  printf("mem id : %d", mem_id);
  stats_t *shm_ptr;
  shm_ptr = shmat(mem_id, (void *)0, 0);


  shm_ptr->pid = getpid();
  shm_ptr->counter = 0;
  shm_ptr->priority = 0;
  shm_ptr->cpu_secs = 0.0;
  printf("Server PID: %d\n", getpid());

  while (shm_ptr->pid == getpid()) {
    printf("Client hasn't hooked up\n");
  }

  printf("Client has hooked up\n");
  printf("Client PID: %d\n", shm_ptr->pid);
  


  /*char *str = "some string";
  strcat(shm_ptr, str);*/
  //printf("shared mem string %s \n", shm_ptr);

  return 0;
}
