#include "sync_data.h"

void test_sync_data();

int main() {
  test_sync_data();

}

void test_sync_data() {
  int *original_data = malloc(256*3*1024);
  for (int i=0; i<254*3*1024/sizeof(int); i++) original_data[i] = i;

  sync_data_t *sync_data = new_sync_data(0, NULL, 256*3*1024, 3*1024);
  for (int i=0; i<256; i++) write_data_frag(sync_data, i, (void *)original_data + 3*1024 * i);

  for (int i=0; i<254*3*1024/sizeof(int); i++) if (original_data[i] == ((int *)sync_data->data)[i]) exit(1);
}