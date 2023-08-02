#include "collab_inference.h"
#include "sync_data.h"

#include <stdio.h>

sync_data_t *new_sync_data(int id, void *data, int data_size, int data_frag_size) {
  sync_data_t *result = (sync_data_t *)malloc(sizeof(sync_data_t));

  result->id = id;

  pthread_mutex_init(&result->mutex_access, NULL);
  pthread_cond_init(&result->cond_finish, NULL);
  
  assert(data_size % data_frag_size == 0);
  result->data = data ? data : malloc(data_size);
  result->data_size = data_size;
  result->data_frag_size = data_frag_size;
  result->num_data_frag = data_size / data_frag_size;

  atomic_store(&result->num_data_frag_ready, 0);

  result->data_frag_ready = (atomic_int *)malloc(sizeof(atomic_int) * result->num_data_frag);
  for (int i=0; i<result->num_data_frag; i++) atomic_store(&result->data_frag_ready[i], 0);

  result->data_frag_access = (atomic_int *)malloc(sizeof(atomic_int) * result->num_data_frag);
  for (int i=0; i<result->num_data_frag; i++) atomic_store(&result->data_frag_access[i], 1);

  result->on_frag_ready = NULL;
  result->on_frag_ready_preargs = NULL;
  result->on_frag_ready_preargs_size = 0;

  result->on_sync_finish = NULL;
  result->on_sync_finish_preargs = NULL;
  result->on_sync_finish_preargs_size = 0;

  return result;
}

void set_handler_sync_data_frag_ready(sync_data_t *sync_data, void (*handler)(void *args), void *preargs, int preargs_size) {
  sync_data->on_frag_ready = handler;
  sync_data->on_frag_ready_preargs = preargs;
  sync_data->on_frag_ready_preargs_size = preargs_size;
}

void set_handler_sync_data_sync_finish(sync_data_t *sync_data, void (*handler)(void *args), void *preargs, int preargs_size) {
  sync_data->on_sync_finish = handler;
  sync_data->on_sync_finish_preargs = preargs;
  sync_data->on_sync_finish_preargs_size = preargs_size;
}

void write_data_frag(sync_data_t *sync_data, int idx, void *data_frag) {
  // Check if already ready
  int is_ready = atomic_load(&sync_data->data_frag_ready[idx]);
  if (is_ready) printf("%d is ready\n", idx);
  if (is_ready) return;

  // Check if someone is writing
  int is_accessible = atomic_exchange(&sync_data->data_frag_access[idx], 0);
  if (!is_accessible) printf("%d is writing\n", idx);
  if (!is_accessible) return;

  printf("writing %d\n", idx);

  // Write data_frag
  memcpy(sync_data->data + sync_data->data_frag_size * idx, data_frag, sync_data->data_frag_size);

  // Manage data_frag completion
  if (sync_data->on_frag_ready != NULL) {
    void *on_frag_ready_args = malloc(sizeof(sync_data_t *) + sizeof(int) + sync_data->on_frag_ready_preargs_size);
    *(sync_data_t **)(on_frag_ready_args) = sync_data;
    *(int *)(on_frag_ready_args + sizeof(sync_data_t *)) = idx;
    memcpy(on_frag_ready_args + sizeof(sync_data_t *) + sizeof(int), sync_data->on_frag_ready_preargs, sync_data->on_frag_ready_preargs_size);

    sync_data->on_frag_ready(on_frag_ready_args);

    free(on_frag_ready_args);
  }
}

int is_data_frag_ready(sync_data_t *sync_data, int idx) {
  return atomic_load(&sync_data->data_frag_ready[idx]);
}