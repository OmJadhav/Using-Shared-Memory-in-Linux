#ifndef stats_h__
#define stats_h__

#define NUM_STATS 16

typedef struct {
  // You may add any new fields that you believe are necessary
  int pid;        // Do not remove or change
  char program_name[16]; // client program name
  int counter;    // Do not remove or change
  int priority;   // Do not remove or change
  double cpu_secs; // Do not remove or change
  // You may add any new fields that you believe are necessary
} stats_t;

extern int test();
extern stats_t* stat_init(key_t key);
extern int stat_unlink(key_t key);
 
#endif