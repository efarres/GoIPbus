/*
 * =====================================================================================
 *
 *       Filename:  buffer.c
 *
 *    Description:  Tests of linear buffer functionality.
 *
 *         Author:  Evan Friis, UW Madison
 *
 * =====================================================================================
 */

#include <string.h>

#include "minunit.h"
#include "buffer.h"

static char* test_buffer_new(void) {
  uint32_t test_data[5] = {0, 1, 2, 3, 4};
  Buffer* mybuf = buffer_new(test_data, 4);
  mu_assert_eq("size", mybuf->size, 4);
  mu_assert_eq("content", memcmp(mybuf->data, test_data, 4 * sizeof(uint32_t)), 0);
  buffer_free(mybuf);
  return 0;
}

static char* test_buffer_free(void) {
  Buffer* mybuf = buffer_new(NULL, 20);
  // doesn't crash
  buffer_free(mybuf);
  return 0;
}

static char* test_buffer_resize(void) {
  uint32_t test_data[5] = {0, 1, 2, 3, 4};
  Buffer* mybuf = buffer_new(test_data, 5);
  buffer_resize(mybuf, 3);
  mu_assert_eq("size", mybuf->size, 3);
  mu_assert_eq("content", 
      memcmp(mybuf->data, test_data, 3 * sizeof(uint32_t)), 0);
  buffer_free(mybuf);
  return 0;
}

int tests_run;

char * all_tests(void) {
  printf("\n\n=== buffer tests ===\n");
  mu_run_test(test_buffer_new);
  mu_run_test(test_buffer_free);
  mu_run_test(test_buffer_resize);
  return 0;
}
