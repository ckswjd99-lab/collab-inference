#include "collab_inference.h"
#include "sync_data.h"

#define NUM_WRITER  3

void *async_write(void *args);

int main() {
  int *original_data = malloc(256*3*1024);
  for (int i=0; i<256*3*1024/sizeof(int); i++) original_data[i] = i;

  sync_data_t *sync_data = new_sync_data(0, NULL, 256*3*1024, 3*1024);
  pthread_t writer[NUM_WRITER];

  void *args = malloc(sizeof(sync_data_t *) + sizeof(int *));
  *(sync_data_t **)args = sync_data;
  *(int **)(args + sizeof(sync_data_t *)) = original_data;

  for (int i=0; i<NUM_WRITER; i++) {
    pthread_create(&writer[i], NULL, async_write, args);
  }

  for (int i=0; i<NUM_WRITER; i++) {
    pthread_join(writer[i], NULL);
  }

  for (int i=0; i<256*3*1024/sizeof(int); i++)
    if (original_data[i] != ((int *)sync_data->data)[i])
      exit(1);
  
  return 0;
}

void *async_write(void *args) {
  sync_data_t *sync_data = *(sync_data_t **)args;
  int *original_data = *(int **)(args + sizeof(sync_data_t *));
  for (int i=0; i<256; i++) {
    write_data_frag(sync_data, i, (void *)original_data + 3*1024 * i);
  }

  return NULL;
}