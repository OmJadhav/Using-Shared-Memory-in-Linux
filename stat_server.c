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
usage(char *prog) 
{
  fprintf(stderr, "usage: %s <-k key> \n", prog);
  exit(1);
}


// signal handler 
void
signal_cleanup(int signum) {
  if (signum == SIGINT) {
  // destroy the shared mem space
  shmctl(mem_id, IPC_RMID, 0);
  
  // remove the semaphore
  char *key_str = malloc(sizeof(char)*(int)log10((double)key)); 
  sprintf(key_str, "%d", key);
  sem_unlink(key_str);
  free(key_str);
  exit(0);
  }
}


int
main(int argc, char *argv[])
{

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
  // printf("key %d \n", key);

  // Initialize the semaphore
  char *key_str = malloc(sizeof(char)*(int)log10((double)key));
  // TO DO : free this 
  sprintf(key_str, "%d", key);
  // printf("key_str %s", key_str);
  sem_t *mutex;
  if ((mutex = sem_open(key_str, O_CREAT, 0644, 1)) == SEM_FAILED) {
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
  printf("page size %d\n",page_size);
  printf("before shmget call\n");
   if ((mem_id = shmget(key, page_size, 0660 | IPC_CREAT)) == -1) {
        perror("shmget");
        exit(1);
    }

  // shmget(key, page_size, IPC_CREAT | 0x666);
  printf("after shmget call\n");
  printf("mem id : %d", mem_id);
  stats_t *shm_ptr;
  shm_ptr = shmat(mem_id, (void *)0, 0);
  stats_t *tmp_ptr = shm_ptr;

  int i;
  for (i = 0; i < NUM_STATS; ++i)
  {
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
      if (tmp_ptr->pid > 0)
      {
        // print pinfo
        printf("%d %d %s %d %d %.2f\n",
          server_counter, tmp_ptr->pid, tmp_ptr->program_name,
            tmp_ptr->counter, tmp_ptr->priority, tmp_ptr->cpu_secs);

      }
      tmp_ptr++;
    }
    server_counter++;
  }
  
  printf("Client has hooked up\n");
  printf("Client PID: %d\n", shm_ptr->pid);
  

  /*char *str = "some string";
  strcat(shm_ptr, str);*/
  //printf("shared mem string %s \n", shm_ptr);

  return 0;
}
