#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <math.h>
#include <string.h>
#include "stats.h"

stats_t*
stats_init(key_t key) {
  // printf("stats init is called: pid: %d \n", getpid());
  sem_t *mutex;
  int ikey = (int) key;
  int size_key = sizeof(char)*(int)log10((double) ikey)
    + strlen("_prewittjadhav");
    char *key_str = malloc(size_key);
  sprintf(key_str, "%d_prewittjadhav", key);
    if ((mutex = sem_open(key_str, 0)) == SEM_FAILED) {
        perror("stats init sem_open");
      free(key_str);
        exit(1);
      }
      free(key_str);


    int page_size = getpagesize();
      int mem_id;
       if ((mem_id = shmget(key, page_size, 0660)) == -1) {
        perror("stats init shmget");
        sem_close(mutex);
        return NULL;
    }
      stats_t *shm_ptr;
      if ((shm_ptr = shmat(mem_id, NULL, 0)) == (void *) -1) {
          perror(" stats_init shmat");
          sem_close(mutex);
          return NULL;
      }

      // sem wait
      if (sem_wait(mutex) == -1) {
        perror("sem_wait");
        exit(1);
      }

      int i;
      int found = 0;
      for (i = 0; i < NUM_STATS; ++i) {
          if (shm_ptr->pid == -1) {
              shm_ptr->pid = -2;
              found = 1;
              break;
          }
          shm_ptr++;
      }
      // TO DO unlock
      if (sem_post(mutex) == -1) {
        perror("sem_post");
        exit(1);
      }

      if (sem_close(mutex) == -1) {
        return NULL;
      }

      if (found) {
          return shm_ptr;
      } else {
          return NULL;
      }
}

int
stats_unlink(key_t key) {
    // find the memeory of the process
    // printf("stats unlink is called: pid: %d \n", getpid());
    int page_size = getpagesize();
    int found = 0;
    int mem_id;
    if ((mem_id = shmget(key, page_size, 0660)) == -1) {
        return -1;
    }
    stats_t *shm_ptr, *found_ptr;
    if ((shm_ptr = shmat(mem_id, (void *)0, 0)) == (void *) -1) {
      return -1;
    }
  int i;
  for (i = 0; i < NUM_STATS; ++i) {
    if (shm_ptr->pid == getpid()) {
      found_ptr = shm_ptr;
      found++;
    }
    shm_ptr++;
  }

  if (found == 0) {
    // no process
    fprintf(stderr, "stat unlink no process");
    return -1;
  } else if (found > 1) {
    // more than one process
    fprintf(stderr, "stat unlink no process");
    return -1;
  } else {
    // found a proc
    // clean
    // lock
    sem_t *mutex;
    int ikey = (int) key;
    int size_key = sizeof(char)*(int)log10((double) ikey)
     + strlen("_prewittjadhav");
    char *key_str = malloc(size_key);
    sprintf(key_str, "%d_prewittjadhav", key);
    if ((mutex = sem_open(key_str, 0)) == SEM_FAILED) {
        free(key_str);
        return -1;
    }
      free(key_str);

      // sem wait
      if (sem_wait(mutex) == -1) {
        return -1;
      }

      found_ptr->pid = -1;
      strcpy(found_ptr->program_name, "");
      found_ptr->counter = 0;
      found_ptr->priority = 0;
      found_ptr->cpu_secs = 0;

    // TO DO unlock
    if (sem_post(mutex) == -1) {
      return -1;
    }
    if (sem_close(mutex) == -1) {
    return -1;
  }
  }
  return 0;
}













