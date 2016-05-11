/*
 * =====================================================================================
 *
 *       Filename:  test_circular_buffer.c
 *
 *    Description:  Tests of buffer functionality.
 *
 *         Author:  Evan Friis, UW Madison
 *
 * =====================================================================================
 */


#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "minunit.h"
#include "circular_buffer.h"

static char* test_cbuffer_new(void) {
  CircularBuffer* mybuf = cbuffer_new();
  mu_assert_eq("size", cbuffer_size(mybuf), 0);
  mu_assert_eq("pos", mybuf->pos, 0);
  mu_assert_eq("freespace", cbuffer_freespace(mybuf), IO_BUFFER_SIZE - 1);
  mu_assert_eq("init is zero", (mybuf->data[0]), 0);

  cbuffer_free(mybuf);

  return 0;
}

static char* test_cbuffer_copy(void) {
  CircularBuffer* mybuf = cbuffer_new();
  mu_assert_eq("size", cbuffer_size(mybuf), 0);
  mu_assert_eq("pos", mybuf->pos, 0);
  mu_assert_eq("tail", mybuf->tail, 0);
  for (uint32_t i = 0; i < IO_BUFFER_SIZE - 2; ++i) {
    cbuffer_push_back(mybuf, i);
  }
  mu_assert_eq("tail", mybuf->tail, IO_BUFFER_SIZE - 2);
  CircularBuffer* copy = cbuffer_copy(mybuf);
  mu_assert_eq("content", memcmp(mybuf->data, copy->data, 
        IO_BUFFER_SIZE * sizeof(uint32_t)), 0);
  mu_assert_eq("pos copy", mybuf->pos, copy->pos);
  mu_assert_eq("tail copy", mybuf->tail, copy->tail);

  cbuffer_free(mybuf);
  cbuffer_free(copy);

  return 0;
}

static char* test_cbuffer_size(void) {
  CircularBuffer* mybuf = cbuffer_new();
  mybuf->pos = IO_BUFFER_SIZE - 5;
  mybuf->tail = IO_BUFFER_SIZE - 5;
  mu_assert_eq("size0", cbuffer_size(mybuf), 0);
  for (int i = 0; i < 15; ++i) {
    cbuffer_push_back(mybuf, i);
    mu_assert_eq("size", cbuffer_size(mybuf), i + 1);
  }

  cbuffer_free(mybuf);

  return 0;
}

static char* test_cbuffer_contiguous_data_size(void) {
  CircularBuffer* mybuf = cbuffer_new();
  mybuf->pos = IO_BUFFER_SIZE - 5;
  mybuf->tail = IO_BUFFER_SIZE - 5;
  mu_assert_eq("size0", cbuffer_contiguous_data_size(mybuf), 0);
  mybuf->tail = 10;
  mu_assert_eq("size5", cbuffer_contiguous_data_size(mybuf), 5);
  mybuf->pos = 3;
  mu_assert_eq("size7", cbuffer_contiguous_data_size(mybuf), 7);

  cbuffer_free(mybuf);

  return 0;
}

static char* test_cbuffer_freespace(void) {
  CircularBuffer* mybuf = cbuffer_new();
  mybuf->tail = IO_BUFFER_SIZE - 5;
  mu_assert_eq("freespace", cbuffer_freespace(mybuf), 4);
  mu_assert_eq("size", cbuffer_size(mybuf), IO_BUFFER_SIZE - 5);
  for (int i = 1; i < 5; ++i) {
    cbuffer_push_back(mybuf, i);
    mu_assert_eq("freespace", cbuffer_freespace(mybuf), 4 - i);
  }

  cbuffer_free(mybuf);

  return 0;
}

static char* test_cbuffer_free(void) {
  CircularBuffer* mybuf = cbuffer_new();
  // doesn't crash
  cbuffer_free(mybuf);
  return 0;
}

static char* test_cbuffer_append(void) {
  CircularBuffer* mybuf = cbuffer_new();
  uint32_t test_data[5] = {0, 1, 2, 3, 4};
  cbuffer_append(mybuf, test_data, 5);
  mu_assert_eq("pos", mybuf->pos, 0);
  mu_assert_eq("size", cbuffer_size(mybuf), 5);
  mu_assert_eq("content", memcmp(mybuf->data, test_data, 5 * sizeof(uint32_t)), 0);

  uint32_t test_data2[3] = {6, 7, 8};
  cbuffer_append(mybuf, test_data2, 3);
  mu_assert_eq("pos", mybuf->pos, 0);
  mu_assert_eq("size", cbuffer_size(mybuf), 8);
  mu_assert_eq("content2", memcmp(mybuf->data, test_data, 5 * sizeof(uint32_t)), 0);

  mu_assert_eq("content3", memcmp(&(mybuf->data[5]), 
        test_data2, 3 * sizeof(uint32_t)), 0);

  cbuffer_free(mybuf);

  return 0;
}

static char* test_cbuffer_append_wraps(void) {
  CircularBuffer* mybuf = cbuffer_new();
  // put us at the end of the buffer
  mybuf->pos = IO_BUFFER_SIZE - 5;
  mybuf->tail = IO_BUFFER_SIZE - 5;
  uint32_t test_data[11] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  cbuffer_append(mybuf, test_data, 11);

  mu_assert_eq("pos", mybuf->pos, IO_BUFFER_SIZE - 5);
  mu_assert_eq("size", cbuffer_size(mybuf), 11);
  mu_assert_eq("tail content", memcmp(
        &(mybuf->data[mybuf->pos]),
        test_data, 
        5 * sizeof(uint32_t)), 0);

  // make sure we aren't trashing the memory after the buffer.
  //mu_assert_eq("tail content sanity", mybuf->data[IO_BUFFER_SIZE-1], 4);

  mu_assert_eq("head content", memcmp(
        mybuf->data, 
        test_data + 5, 
        6 * sizeof(uint32_t)), 0);

  uint32_t test_data2[3] = {11, 12, 13};
  cbuffer_append(mybuf, test_data2, 3);
  mu_assert_eq("size", cbuffer_size(mybuf), 14);
  mu_assert_eq("content", memcmp(&(mybuf->data[6]), test_data2, 3), 0);

  cbuffer_free(mybuf);

  return 0;
}

static char* test_cbuffer_read(void) {
  CircularBuffer* mybuf = cbuffer_new();
  Buffer* readout = buffer_new(NULL, 11);
  uint32_t test_data[11] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  cbuffer_append(mybuf, test_data, 11);
  cbuffer_read(mybuf, readout->data, 11);
  mu_assert_eq("size", readout->size, 11);
  mu_assert_eq("content", memcmp(readout->data, test_data, 11 * sizeof(uint32_t)), 0);
  // if we ask for too much it cbuffer gives us what it has.
  Buffer* readout2 = buffer_new(NULL, 30);
  int actually_read = cbuffer_read(mybuf, readout2->data, 30);
  mu_assert_eq("size2", actually_read, 11);
  mu_assert_eq("content2", memcmp(readout2->data, test_data, 11 * sizeof(uint32_t)), 0);

  cbuffer_free(mybuf);
  buffer_free(readout);
  buffer_free(readout2);

  return 0;
}

static char* test_cbuffer_read_wraps(void) {
  CircularBuffer* mybuf = cbuffer_new();
  // put us at the end of the buffer
  mybuf->pos = IO_BUFFER_SIZE - 5;
  mybuf->tail = IO_BUFFER_SIZE - 5;
  uint32_t test_data[11] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  cbuffer_append(mybuf, test_data, 11);
  Buffer* readout = buffer_new(NULL, 11);
  cbuffer_read(mybuf, readout->data, 11);
  mu_assert_eq("size", readout->size, 11);
  mu_assert_eq("content", memcmp(readout->data, test_data, 11 * sizeof(uint32_t)), 0);

  cbuffer_free(mybuf);
  buffer_free(readout);

  return 0;
}

static char* test_cbuffer_value_at_wraps(void) {
  CircularBuffer* mybuf = cbuffer_new();
  // put us at the end of the buffer
  mybuf->pos = IO_BUFFER_SIZE - 5;
  mybuf->tail = IO_BUFFER_SIZE - 5;
  uint32_t test_data[11] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  cbuffer_append(mybuf, test_data, 11);
  for (int i = 0; i < 11; ++i) {
    mu_assert_eq("read at", cbuffer_value_at(mybuf, i), test_data[i]);
  }

  cbuffer_free(mybuf);

  return 0;
}

static char* test_cbuffer_delete_front(void) {
  CircularBuffer* mybuf = cbuffer_new();
  uint32_t test_data[11] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  cbuffer_append(mybuf, test_data, 11);
  mu_assert_eq("content", memcmp(mybuf->data, test_data, 11 * sizeof(uint32_t)), 0);
  int deleted = cbuffer_deletefront(mybuf, 5);
  mu_assert_eq("deleted", deleted, 5);
  mu_assert_eq("pos", mybuf->pos, 5);
  mu_assert_eq("size", cbuffer_size(mybuf), 6);

  mu_assert_eq("item0", mybuf->data[mybuf->pos], 5);
  mu_assert_eq("item1", mybuf->data[mybuf->pos+1], 6);
  mu_assert_eq("item2", mybuf->data[mybuf->pos+2], 7);

  mu_assert_eq("remaining content in cbuffer", memcmp(
        &(mybuf->data[5]), 
        test_data + 5, 6 * sizeof(uint32_t)), 0);

  Buffer* readout = buffer_new(NULL, 6);
  cbuffer_read(mybuf, readout->data, 6);
  mu_assert_eq("remaining content", memcmp(
        readout->data, 
        (test_data + 5), 
        6 * sizeof(uint32_t)), 0);

  // if we ask to delete everything, just return what was actually deleted.
  int deleted_just_to_end = cbuffer_deletefront(mybuf, 100);
  mu_assert_eq("deleted just to end", deleted_just_to_end, 6);
  mu_assert_eq("pos2", mybuf->pos, 11);
  mu_assert_eq("size2", cbuffer_size(mybuf), 0);

  cbuffer_free(mybuf);
  buffer_free(readout);

  return 0;
}

static char* test_cbuffer_delete_front_wraps(void) {
  CircularBuffer* mybuf = cbuffer_new();
  uint32_t test_data[11] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  // put us at the end of the buffer
  mybuf->pos = IO_BUFFER_SIZE - 5;
  mybuf->tail = IO_BUFFER_SIZE - 5;
  cbuffer_append(mybuf, test_data, 11);
  mu_assert_eq("content", memcmp(&(mybuf->data[mybuf->pos]), 
        test_data, 5 * sizeof(uint32_t)), 0);
  int deleted = cbuffer_deletefront(mybuf, 5);
  mu_assert_eq("deleted", deleted, 5);
  mu_assert_eq("pos", mybuf->pos, 0);
  mu_assert_eq("size", cbuffer_size(mybuf), 6);

  mu_assert_eq("item0", mybuf->data[mybuf->pos], 5);
  mu_assert_eq("item1", mybuf->data[mybuf->pos+1], 6);
  mu_assert_eq("item2", mybuf->data[mybuf->pos+2], 7);

  mu_assert_eq("remaining content in cbuffer", memcmp(
        &(mybuf->data[0]), 
        test_data + 5, 6 * sizeof(uint32_t)), 0);

  Buffer* readout = buffer_new(NULL, 6);
  cbuffer_read(mybuf, readout->data, 6);
  mu_assert_eq("remaining content", memcmp(
        readout->data, 
        (test_data + 5), 
        6 * sizeof(uint32_t)), 0);

  // if we ask to delete everything, just return what was actually deleted.
  int deleted_just_to_end = cbuffer_deletefront(mybuf, 100);
  mu_assert_eq("deleted just to end", deleted_just_to_end, 6);
  mu_assert_eq("pos2", mybuf->pos, 6);
  mu_assert_eq("size2", cbuffer_size(mybuf), 0);

  cbuffer_free(mybuf);
  buffer_free(readout);

  return 0;
}


static char* test_cbuffer_pop(void) {
  CircularBuffer* mybuf = cbuffer_new();
  // put us at the end of the buffer
  mybuf->pos = IO_BUFFER_SIZE - 5;
  mybuf->tail = IO_BUFFER_SIZE - 5;
  uint32_t test_data[11] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  cbuffer_append(mybuf, test_data, 11);

  Buffer* bucky = cbuffer_pop(mybuf, 5);
  mu_assert_eq("size", cbuffer_size(mybuf), 6);
  mu_assert_eq("content", memcmp(bucky->data, test_data, 
        5*sizeof(uint32_t)), 0);

  Buffer* badger = cbuffer_pop(mybuf, 6);
  mu_assert_eq("size2", cbuffer_size(mybuf), 0);
  mu_assert_eq("content2", 
      memcmp(badger->data, test_data + 5, 6), 0);

  // if we pop an empty collection, we get nothing.
  Buffer* empty = cbuffer_pop(mybuf, 10);
  mu_assert_eq("size3", empty->size, 0);

  cbuffer_free(mybuf);
  buffer_free(bucky);
  buffer_free(badger);
  buffer_free(empty);

  return 0;
}

static char* test_cbuffer_pop_front(void) {
  CircularBuffer* mybuf = cbuffer_new();
  // put us at the end of the buffer
  mybuf->pos = IO_BUFFER_SIZE - 5;
  mybuf->tail = IO_BUFFER_SIZE - 5;
  uint32_t test_data[11] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  cbuffer_append(mybuf, test_data, 11);

  for (int i = 0; i < 11; i++) {
    mu_assert_eq("pre-pop size", cbuffer_size(mybuf), 11 - i);
    uint32_t value = cbuffer_pop_front(mybuf);
    mu_assert_eq("popped_content", (int)value, i);
  }

  // if we pop an empty collection, we get dead beef.
  uint32_t empty = cbuffer_pop_front(mybuf);
  mu_assert_eq("empty", empty, 0xDEADBEEF);

  cbuffer_free(mybuf);

  return 0;
}

static char* test_cbuffer_push_back(void) {
  CircularBuffer* mybuf = cbuffer_new();
  // put us at the end of the buffer
  mybuf->pos = IO_BUFFER_SIZE - 2;
  mybuf->tail = IO_BUFFER_SIZE - 2;
  cbuffer_push_back(mybuf, 0xDEADBEEF);
  cbuffer_push_back(mybuf, 0xBEEFFACE);
  cbuffer_push_back(mybuf, 0xDEADFACE);

  mu_assert_eq("size", cbuffer_size(mybuf), 3);
  mu_assert_eq("pos", mybuf->pos, IO_BUFFER_SIZE-2);

  mu_assert_eq("item0", mybuf->data[mybuf->pos], 0xDEADBEEF);
  mu_assert_eq("item1", mybuf->data[mybuf->pos + 1], 0xBEEFFACE);
  mu_assert_eq("item2", mybuf->data[0], 0xDEADFACE);

  cbuffer_free(mybuf);

  return 0;
}

static char* test_cbuffer_net_features(void) {
  CircularBuffer* mybuf = cbuffer_new();
  // put us at the end of the buffer
  cbuffer_push_back_net(mybuf, 0xDEADBEEF);
  cbuffer_push_back_net(mybuf, 0xBEEFFACE);
  cbuffer_push_back_net(mybuf, 0xDEADFACE);

  mu_assert_eq("item0", cbuffer_value_at_net(mybuf, 0), 0xDEADBEEF);
  mu_assert_eq("item1", cbuffer_value_at_net(mybuf, 1), 0xBEEFFACE);
  mu_assert_eq("item2", cbuffer_value_at_net(mybuf, 2), 0xDEADFACE);

  cbuffer_free(mybuf);

  return 0;
}

static char* test_cbuffer_fd_features(void) {
  // make pipes 
  int pipefd[2];
  pipe(pipefd);

  // make txpipe nonblocking, so we can check if it's empty.
  fcntl(pipefd[0], F_SETFL, fcntl(pipefd[0], F_GETFL) | O_NONBLOCK);

  int in = pipefd[1];
  int out = pipefd[0];

  CircularBuffer* frombuf = cbuffer_new();
  for (int i = 0; i < 200; ++i) {
    cbuffer_push_back(frombuf, i);
  }
  mu_assert_eq("from size", cbuffer_size(frombuf), 200);

  CircularBuffer* tobuf = cbuffer_new();
  tobuf->pos = IO_BUFFER_SIZE - 100;
  tobuf->tail = IO_BUFFER_SIZE - 100;

  ssize_t written = cbuffer_write_fd(frombuf, in, 200);
  mu_assert_eq("wrote to pipe", written, 200);
  mu_assert_eq("from size after", cbuffer_size(frombuf), 0);
  ssize_t read = cbuffer_read_fd(tobuf, out, 200);
  mu_assert_eq("read from pipe", read, 200);

  for (int i = 0; i < 200; ++i) {
    mu_assert_eq("fd closure value", cbuffer_value_at(tobuf, i), i);
  }

  cbuffer_free(frombuf);
  cbuffer_free(tobuf);

  return 0;
}

static char* test_cbuffer_fd_full(void) {
  // make sure we can stop reading if our read buffer is full
  // make pipes 
  int pipefd[2];
  pipe(pipefd);

  // make txpipe nonblocking, so we can check if it's empty.
  fcntl(pipefd[0], F_SETFL, fcntl(pipefd[0], F_GETFL) | O_NONBLOCK);

  int in = pipefd[1];
  int out = pipefd[0];

  CircularBuffer* frombuf = cbuffer_new();
  for (int i = 0; i < 200; ++i) {
    cbuffer_push_back(frombuf, i);
  }
  mu_assert_eq("from size", cbuffer_size(frombuf), 200);

  CircularBuffer* tobuf = cbuffer_new();
  tobuf->pos = IO_BUFFER_SIZE - 100;
  tobuf->tail = IO_BUFFER_SIZE - 100;

  ssize_t written = cbuffer_write_fd(frombuf, in, 200);
  mu_assert_eq("wrote to pipe", written, 200);
  mu_assert_eq("from size after", cbuffer_size(frombuf), 0);
  tobuf->tail += IO_BUFFER_SIZE - 100;
  mu_assert_eq("to freespace", cbuffer_freespace(tobuf), 99);
  ssize_t read = cbuffer_read_fd(tobuf, out, 200);
  ssize_t exp = 99;
  mu_assert_eq("read from pipe", (int)read, (int)exp);

  cbuffer_free(frombuf);
  cbuffer_free(tobuf);

  return 0;
}

/*
 * Make sure cbuffer_fd_read handles the edge case where cbuffer->tail returns
 * to the front of buffer->data
 */
static char* test_cbuffer_fd_read_edge(void) {
  int pipefd[2];
  pipe(pipefd);

  fcntl(pipefd[0], F_SETFL, fcntl(pipefd[0], F_GETFL) | O_NONBLOCK);

  int in = pipefd[1];
  int out = pipefd[0];

  CircularBuffer* mybuf = cbuffer_new();

  // completely fill buffer
  while (cbuffer_freespace(mybuf) > 0) {
    mu_assert_eq("mybuf overflow", cbuffer_push_back(mybuf, 0xDEADBEEF), 0);
  }
  mu_assert_eq("mybuf still has free space", cbuffer_freespace(mybuf), 0);

  // clear a single space in the cbuffer
  // free space should be at the front of cbuffer->data
  cbuffer_deletefront(mybuf, 1);

  uint32_t inbuf[] = {0xCAFEBABE};
  write(in, inbuf, sizeof(uint32_t));

  // read a single word into the free slot in the buffer
  mu_assert_eq("Could not read data", cbuffer_read_fd(mybuf, out, 1), 1);

  // check the content of the cbuffer
  while (cbuffer_size(mybuf) > 1) {
    mu_assert_eq("Content should be 0xDEADBEEF",
        cbuffer_pop_front(mybuf), 0xDEADBEEF);
  }
  mu_assert_eq("Content should be 0xCAFEBABE",
      cbuffer_pop_front(mybuf), 0xCAFEBABE);

  cbuffer_free(mybuf);
  return 0;
}

static char* test_cbuffer_transfer_data(void) {
  CircularBuffer* src = cbuffer_new();
  CircularBuffer* dst = cbuffer_new();

  for (int i = 0; i < 200; ++i) {
    cbuffer_push_back(src, i);
  }
  mu_assert_eq("Src buffer size", cbuffer_size(src), 200);
  mu_assert_eq("Dst buffer size", cbuffer_size(dst), 0);

  // transfer data from src -> dst
  cbuffer_transfer_data(src, dst);

  mu_assert_eq("Src buffer size post transfer",
      cbuffer_size(src), 0);
  mu_assert_eq("Dst buffer size post transfer",
      cbuffer_size(dst), 200);

  // check content
  for (int i = 0; i < 200; ++i) {
    mu_assert_eq("Dst content",
        cbuffer_pop_front(dst), i);
  }

  cbuffer_free(src);
  cbuffer_free(dst);

  return 0;
}

int tests_run;

char * all_tests(void) {
  printf("\n\n=== circular buffer tests ===\n");
  mu_run_test(test_cbuffer_new);
  mu_run_test(test_cbuffer_copy);
  mu_run_test(test_cbuffer_size);
  mu_run_test(test_cbuffer_contiguous_data_size);
  mu_run_test(test_cbuffer_freespace);
  mu_run_test(test_cbuffer_free);
  mu_run_test(test_cbuffer_append);
  mu_run_test(test_cbuffer_append_wraps);
  mu_run_test(test_cbuffer_read);
  mu_run_test(test_cbuffer_read_wraps);
  mu_run_test(test_cbuffer_value_at_wraps);
  mu_run_test(test_cbuffer_delete_front);
  mu_run_test(test_cbuffer_delete_front_wraps);
  mu_run_test(test_cbuffer_pop);
  mu_run_test(test_cbuffer_pop_front);
  mu_run_test(test_cbuffer_push_back);
  mu_run_test(test_cbuffer_net_features);
  mu_run_test(test_cbuffer_fd_features);
  mu_run_test(test_cbuffer_fd_full);
  mu_run_test(test_cbuffer_fd_read_edge);
  mu_run_test(test_cbuffer_transfer_data);
  return 0;
}
