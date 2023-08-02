#include "collab_inference.h"
#include "sync_data.h"
#include "utils.h"
#include <stdio.h>
#include <time.h>

#define NUM_WRITER  2
#define FRAG_SIZE   (3*1024)
#define NUM_FRAG    256
#define DATA_SIZE   (FRAG_SIZE * NUM_FRAG)

void *async_write(void *args);
void *print_ready(void *args);
void *print_complete(void *args);

int main() {
  int *original_data = malloc(DATA_SIZE);
  for (int i=0; i<DATA_SIZE/sizeof(int); i++) original_data[i] = i;

  sync_data_t *sync_data = new_sync_data(0, NULL, DATA_SIZE, FRAG_SIZE);
  pthread_t writer[NUM_WRITER];
  void *args_arr[NUM_WRITER];

  set_handler_sync_data_frag_ready(sync_data, print_ready, NULL, 0);
  set_handler_sync_data_sync_finish(sync_data, print_complete, NULL, 0);

  unsigned int start_time, end_time;
  start_time = get_usec();

  for (int i=0; i<NUM_WRITER; i++) {
    args_arr[i] = malloc(sizeof(sync_data_t *) + sizeof(int *) + sizeof(int));
    *(sync_data_t **)args_arr[i] = sync_data;
    *(int **)(args_arr[i] + sizeof(sync_data_t *)) = original_data;
    *(int *)(args_arr[i] + sizeof(sync_data_t *) + sizeof(int *)) = i;
    pthread_create(&writer[i], NULL, async_write, args_arr[i]);
  }

  for (int i=0; i<NUM_WRITER; i++) {
    pthread_join(writer[i], NULL);
    free(args_arr[i]);
  }

  end_time = get_usec();

  printf("Elapsed time: %u(ms)\n", end_time - start_time);

  for (int i=0; i<DATA_SIZE/sizeof(int); i++)
    if (original_data[i] != ((int *)sync_data->data)[i])
      exit(1);
  
  return 0;
}

void *async_write(void *args) {
  sync_data_t *sync_data = *(sync_data_t **)args;
  int *original_data = *(int **)(args + sizeof(sync_data_t *));
  int writer_idx = *(int *)(args + sizeof(sync_data_t *) + sizeof(int *));
  for (int i=0; i<NUM_FRAG; i++) {
    int i_modified = (NUM_FRAG / NUM_WRITER * writer_idx + i) % NUM_FRAG;
    write_data_frag(sync_data, i_modified, (void *)original_data + FRAG_SIZE * i_modified);
    if (is_sync_finish(sync_data)) {
      return NULL;
    }
  }

  return NULL;
}

void *print_ready(void *args) {
  sync_data_t *sync_data = *(sync_data_t **)args;
  int frag_idx = *(int *)(args + sizeof(sync_data_t *));
  printf("%d ready\n", frag_idx);
  return NULL;
}

void *print_complete(void *args) {
  sync_data_t *sync_data = *(sync_data_t **)args;
  int frag_idx = *(int *)(args + sizeof(sync_data_t *));
  printf("complete!\n");
  return NULL;
}