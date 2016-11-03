#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <math.h>
#include "stats.h"

int key = 0;
int mem_id = -1;

void
usage(char *prog) {
  fprintf(stderr, "usage: %s -k key \n", prog);
  exit(1);
}


// signal handler
void
signal_cleanup(int signum) {
  if (signum == SIGINT) {
  // destroy the shared mem space
  shmctl(mem_id, IPC_RMID, 0);

  // remove the semaphore
  int ikey = (int) key;
  int size_key = sizeof(char)*(int)log10((double) ikey)
    + strlen("_prewittjadhav");
  char *key_str = malloc(size_key);
  sprintf(key_str, "%d_prewittjadhav", key);
  sem_unlink(key_str);
  free(key_str);
  // printf("server shutting down");
  exit(0);
  }
}


int
main(int argc, char *argv[]) {
  // signal structure
  struct sigaction action;
  // input params
  int c;
  opterr = 0;
  if (argc != 3) {
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
  // printf("key %d \n", key);

  // Initialize the semaphore
  int ikey = (int) key;
  int size_key = sizeof(char)*(int)log10((double) ikey)
    + strlen("_prewittjadhav");
  char *key_str = malloc(size_key);
  sprintf(key_str, "%d_prewittjadhav", key);
  sem_t *mutex;
  if ((mutex = sem_open(key_str, O_CREAT | O_EXCL, 0644, 1)) == SEM_FAILED) {
    perror("sem_open");
    free(key_str);
    exit(1);
  }
  free(key_str);
  // handle signal
  action.sa_handler = signal_cleanup;
  // sigemptyset(&action.sa_mask);
  sigaction(SIGINT, &action, NULL);

  // Shared mem create
  int page_size = getpagesize();
  if ((mem_id = shmget(key, page_size, 0660 | IPC_CREAT)) == -1) {
        perror("server shmget");
        exit(1);
  }

  // shmget(key, page_size, IPC_CREAT | 0x666);
  stats_t *shm_ptr;
  if ((shm_ptr = shmat(mem_id, (void *)0, 0)) == (void*)-1) {
    perror("shmat server\n");
    exit(1);
  }
  stats_t *tmp_ptr = shm_ptr;

  int i;
  for (i = 0; i < NUM_STATS; ++i) {
      tmp_ptr->pid = -1;
      tmp_ptr++;
  }

  /*
  shm_ptr->pid = getpid();
  shm_ptr->counter = 0;
  shm_ptr->priority = 0;
  shm_ptr->cpu_secs = 0.0;
  */
  // printf("Server PID: %d\n", getpid());

  int server_counter = 1;
  while (1) {
    sleep(1);
    int i;
    tmp_ptr = shm_ptr;
    for (i = 0; i < NUM_STATS; ++i) {
      if (tmp_ptr->pid > 0) {
        // print pinfo
        printf("%d %d %s %d %.2f %d\n",
          server_counter, tmp_ptr->pid, tmp_ptr->program_name,
            tmp_ptr->counter, tmp_ptr->cpu_secs, tmp_ptr->priority);
      }
      tmp_ptr++;
    }
    printf("\n");
    server_counter++;
  }
  return 0;
}
