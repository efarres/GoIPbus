/*
 * 
 * Tests of ByteBuffer and its helpers.
 *
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "minunit.h"
#include "bytebuffer.h"

static char* test_bytebuffer_ctor(void) {
  char * buckysname = "BuckyBadger";
  ByteBuffer mybuf = bytebuffer_ctor((unsigned char*)buckysname, 5);
  mu_assert_eq("bufsize", mybuf.bufsize, 5);
  mu_assert("bufcontent", memcmp(mybuf.buf, buckysname, 5)==0);
  return 0;
}

static char* test_bytebuffer_null_ctor(void) {
  char * buckysname = "BuckyBadger";
  // size zero
  ByteBuffer mybuf = bytebuffer_ctor((unsigned char*)buckysname, 0);
  mu_assert_eq("bufsize", mybuf.bufsize, 0);
  mu_assert("buf is null", mybuf.buf == NULL);
  return 0;
}

static char* test_bytebuffer_calloc(void) {
  ByteBuffer mybuf = bytebuffer_ctor(NULL, 5);
  mu_assert_eq("bufsize", mybuf.bufsize, 5);
  unsigned char * zeros = calloc(5, 1);
  mu_assert("bufcontent", memcmp(mybuf.buf, zeros, 5)==0);
  return 0;
}

static char* test_bytebuffer_free(void) {
  char * buckysname = "BuckyBadger";
  ByteBuffer mybuf = bytebuffer_ctor((unsigned char*)buckysname, 5);
  bytebuffer_free(&mybuf);
  mu_assert_eq("bufsize", mybuf.bufsize, 0);
  mu_assert("bufcontent", mybuf.buf == NULL);
  return 0;
}

static char* test_bytebuffer_reserve_back(void) {
  char * buckysname = "BuckyBadger";
  ByteBuffer mybuf = bytebuffer_ctor((unsigned char*)buckysname, 5);
  mu_assert_eq("bufsize", mybuf.bufsize, 5);
  mu_assert("bufcontent", memcmp(mybuf.buf, buckysname, 5)==0);
  bytebuffer_reserve_back(&mybuf, 6);
  mu_assert_eq("bufsize", mybuf.bufsize, 11);
  mu_assert("bufcontent", memcmp(mybuf.buf, buckysname, 5)==0);
  return 0;
}

static char* test_bytebuffer_reserve_back_empty(void) {
  char * buckysname = "BuckyBadger";
  ByteBuffer mybuf = bytebuffer_ctor((unsigned char*)buckysname, 5);
  bytebuffer_free(&mybuf);
  mu_assert_eq("bufsize", mybuf.bufsize, 0);
  mu_assert("bufisNULL", mybuf.buf == NULL);
  bytebuffer_reserve_back(&mybuf, 6);
  mu_assert_eq("bufsize", mybuf.bufsize, 6);
  mu_assert("bufisNotNULL", mybuf.buf != NULL);
  return 0;
}

static char* test_bytebuffer_append(void) {
  char * buckysname = "BuckyBadger";
  ByteBuffer mybuf = bytebuffer_ctor((unsigned char*)buckysname, 5);
  mu_assert_eq("bufsize", mybuf.bufsize, 5);
  mu_assert("bufcontent", memcmp(mybuf.buf, buckysname, 5)==0);
  bytebuffer_append(&mybuf, (unsigned char*)buckysname + 5, 6);
  mu_assert_eq("bufsize", mybuf.bufsize, 11);
  mu_assert("bufcontent", memcmp(mybuf.buf, buckysname, 11)==0);
  return 0;
}

// appending to an uninitizlied buffer should be just like ctor
static char* test_bytebuffer_empty_append(void) {
  char * buckysname = "BuckyBadger";
  ByteBuffer mybuf = bytebuffer_ctor((unsigned char*)buckysname, 5);
  bytebuffer_free(&mybuf);
  mu_assert_eq("bufsize", mybuf.bufsize, 0);
  mu_assert("bufisNULL", mybuf.buf == NULL);
  bytebuffer_append(&mybuf, (unsigned char*)buckysname + 5, 6);
  mu_assert_eq("bufsize", mybuf.bufsize, 6);
  mu_assert("bufNotNULL", mybuf.buf != NULL);
  mu_assert_eq("bufsize", mybuf.bufsize, 6);
  mu_assert("bufcontent", memcmp(mybuf.buf, buckysname+5, 6)==0);
  return 0;
}

// appending zeros to an existing buffer
static char* test_bytebuffer_append_zeros(void) {
  char * buckysname = "BuckyBadger";
  ByteBuffer mybuf = bytebuffer_ctor((unsigned char*)buckysname, 5);
  mu_assert_eq("bufsize", mybuf.bufsize, 5);
  mu_assert("bufcontent", memcmp(mybuf.buf, buckysname, 5)==0);

  bytebuffer_append(&mybuf, NULL, 6);
  mu_assert_eq("bufsize", mybuf.bufsize, 11);
  unsigned char zeros[6] = {0, 0, 0, 0, 0, 0};
  mu_assert("buf content zeros", memcmp(mybuf.buf + 5, zeros, 6)==0);
  mu_assert("bucky content", memcmp(mybuf.buf, buckysname, 5)==0);
  return 0;
}

static char* test_bytebuffer_pop(void) {
  char * buckysname = "BuckyBadger";
  ByteBuffer mybuf = bytebuffer_ctor((unsigned char*)buckysname, 11);
  mu_assert_eq("bufsize", mybuf.bufsize, 11);
  mu_assert("bufcontent", memcmp(mybuf.buf, buckysname, 11)==0);

  ByteBuffer firstname = bytebuffer_pop(&mybuf, 5);

  mu_assert_eq("bufsize", mybuf.bufsize, 6);
  mu_assert("bufcontent", memcmp(mybuf.buf, buckysname+5, 6)==0);

  mu_assert_eq("popped size", firstname.bufsize, 5);
  mu_assert("popped content", memcmp(firstname.buf, buckysname, 5)==0);
  return 0;
}

static char* test_bytebuffer_del_back(void) {
  char * buckysname = "BuckyBadger";
  ByteBuffer mybuf = bytebuffer_ctor((unsigned char*)buckysname, 11);
  mu_assert_eq("bufsize", mybuf.bufsize, 11);
  mu_assert("bufcontent", memcmp(mybuf.buf, buckysname, 11)==0);

  bytebuffer_del_back(&mybuf, 6);

  mu_assert_eq("bufsize", mybuf.bufsize, 5);
  mu_assert("bufcontent", memcmp(mybuf.buf, buckysname, 5)==0);

  return 0;
}

static char* test_bytebuffer_del_back_all(void) {
  // this should remove everythign 
  char * buckysname = "BuckyBadger";
  ByteBuffer mybuf = bytebuffer_ctor((unsigned char*)buckysname, 11);
  mu_assert_eq("bufsize", mybuf.bufsize, 11);
  mu_assert("bufcontent before", memcmp(mybuf.buf, buckysname, 11)==0);

  bytebuffer_del_back(&mybuf, 20);

  mu_assert_eq("bufsize", mybuf.bufsize, 0);
  mu_assert("bufIsNull", mybuf.buf == NULL);

  return 0;
}

static char* test_bytebuffer_del_front(void) {
  char * buckysname = "BuckyBadger";
  ByteBuffer mybuf = bytebuffer_ctor((unsigned char*)buckysname, 11);
  mu_assert_eq("bufsize", mybuf.bufsize, 11);
  mu_assert("bufcontent before", memcmp(mybuf.buf, buckysname, 11)==0);

  bytebuffer_del_front(&mybuf, 5);

  mu_assert_eq("bufsize", mybuf.bufsize, 6);
  mu_assert("bufcontent", memcmp(mybuf.buf, buckysname+5, 6)==0);

  return 0;
}

static char* test_bytebuffer_del_front_all(void) {
  // this should remove everythign 
  char * buckysname = "BuckyBadger";
  ByteBuffer mybuf = bytebuffer_ctor((unsigned char*)buckysname, 11);
  mu_assert_eq("bufsize", mybuf.bufsize, 11);
  mu_assert("bufcontent before", memcmp(mybuf.buf, buckysname, 11)==0);

  bytebuffer_del_front(&mybuf, 20);

  mu_assert_eq("bufsize", mybuf.bufsize, 0);
  mu_assert("bufIsNull", mybuf.buf == NULL);

  return 0;
}

static char* test_bytebuffer_read_fd(void) {
  char * buckysname = "BuckyBadger";
  ByteBuffer mybuf = bytebuffer_ctor((unsigned char*)buckysname, 11);
  mu_assert_eq("bufsize", mybuf.bufsize, 11);
  mu_assert("bufcontent before", memcmp(mybuf.buf, buckysname, 11)==0);

  char * cheeseco = "CheeseCo";

  // make pipes 
  int pipefd[2];
  pipe(pipefd);

  // make txpipe nonblocking, so we can check if it's empty.
  fcntl(pipefd[0], F_SETFL, fcntl(pipefd[0], F_GETFL) | O_NONBLOCK);

  int in = pipefd[1];
  int out = pipefd[0];

  write(in, cheeseco, 8);

  int bytes_read = bytebuffer_read_fd(&mybuf, out, 1000);

  mu_assert_eq("bufsize", mybuf.bufsize, 11 + 8);
  mu_assert_eq("bytes_read", bytes_read, 8);

  char * expected = "BuckyBadgerCheeseCo";

  mu_assert("bufcontent", memcmp(mybuf.buf, expected, 19)==0);
  return 0;
}

int tests_run;

char * all_tests(void) {
  printf("\n\n=== test_bytebuffer ===\n");
  mu_run_test(test_bytebuffer_ctor);
  mu_run_test(test_bytebuffer_null_ctor);
  mu_run_test(test_bytebuffer_calloc);
  mu_run_test(test_bytebuffer_free);
  mu_run_test(test_bytebuffer_reserve_back);
  mu_run_test(test_bytebuffer_reserve_back_empty);
  mu_run_test(test_bytebuffer_append);
  mu_run_test(test_bytebuffer_empty_append);
  mu_run_test(test_bytebuffer_append_zeros);
  mu_run_test(test_bytebuffer_pop);
  mu_run_test(test_bytebuffer_del_back);
  mu_run_test(test_bytebuffer_del_back_all);
  mu_run_test(test_bytebuffer_del_front);
  mu_run_test(test_bytebuffer_del_front_all);
  mu_run_test(test_bytebuffer_read_fd);
  return 0;
}
