#include "collab_inference.h"
#include "pthread.h"
#include "stdatomic.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define DF_EMPTY  0
#define DF_READY  1

#define DF_NONOCCUP  0
#define DF_OCCUP  1

struct sync_data_t {
  int id;
  
  pthread_mutex_t mutex_access;
  pthread_cond_t cond_finish;

  void *data;
  int data_size;
  int data_frag_size;
  int num_data_frag;
  atomic_int num_data_frag_ready;
  atomic_int *data_frag_ready;
  atomic_int *data_frag_access;

  void (*on_frag_ready)(void *args);  // call convention: on_frag_ready(sync_data_t *this, int data_frag_idx, void *on_frag_ready_preargs)
  void *on_frag_ready_preargs;
  int on_frag_ready_preargs_size;

  void (*on_sync_finish)(void *args);   // call convention: on_sync_finish(sync_data_t *this, int data_frag_idx, void *on_sync_finish_preargs)
  void *on_sync_finish_preargs;
  int on_sync_finish_preargs_size;
};

sync_data_t *new_sync_data(int id, void *data, int data_size, int data_frag_size);
void set_handler_sync_data_frag_ready(sync_data_t *sync_data, void (*handler)(void *args), void *preargs, int preargs_size);
void set_handler_sync_data_sync_finish(sync_data_t *sync_data, void (*handler)(void *args), void *preargs, int preargs_size);

void write_data_frag(sync_data_t *sync_data, int idx, void *data_frag);
int is_data_frag_ready(sync_data_t *sync_data, int idx);