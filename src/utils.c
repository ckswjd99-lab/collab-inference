#include "utils.h"

unsigned int get_usec() {
  struct timeval current_time;
	gettimeofday(&current_time, NULL);
  return current_time.tv_sec * (int)1e6 + current_time.tv_usec;
}