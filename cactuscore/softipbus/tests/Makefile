# Makefile for softipbus unit tests.  Run 'make all' to build and run all tests.
#
# Logging level: 0-supress, 1-error, 2-info, 3-debug
ifndef LOG_LEVEL
LOG_LEVEL=0x2
endif

CC=gcc
CCOPTS=-ggdb -std=gnu99 -Wall -Werror -DLOG_LEVEL=$(LOG_LEVEL)
INCLUDES=-I../include 

COMPILE_TEST=$(CC) $(CCOPTS) $(INCLUDES) minunit.c -o $@

test_bytebuffer.exe: bytebuffer.c ../src/bytebuffer.c
	$(COMPILE_TEST) $^

test_buffer.exe: buffer.c ../src/buffer.c
	$(COMPILE_TEST) $^

test_circular_buffer.exe: circular_buffer.c ../src/circular_buffer.c ../src/buffer.c
	$(COMPILE_TEST) $^

BUFFER_TESTS=test_bytebuffer.exe test_buffer.exe test_circular_buffer.exe

BUFFER_SRCS=../src/bytebuffer.c ../src/circular_buffer.c ../src/buffer.c
CORE_SRCS=../src/serialization.c ../src/packethandler.c 
LOCAL_SRV_SRCS=../src/transactionhandler.c ../src/handlers.c 
FORWARDING_SRCS=../src/forwardingtransactionhandler.c \
		../src/nullmembase.c ../src/tty_functions.c

test_forwarding.exe: forwardingtransactionhandler.c \
  $(BUFFER_SRCS) $(CORE_SRCS) $(FORWARDING_SRCS)
	$(COMPILE_TEST) $^

test_packethandler.exe: packethandler.c \
  $(BUFFER_SRCS) $(CORE_SRCS) ../src/transactionhandler.c ../src/dummyhandlers.c
	$(COMPILE_TEST) $^

test_serialization.exe: serialization.c $(BUFFER_SRCS) $(CORE_SRCS) \
  $(LOCAL_SRV_SRCS) ../src/testmembase.c
	$(COMPILE_TEST) $^

IPBUS_TESTS=test_forwarding.exe test_packethandler.exe test_serialization.exe

all: $(BUFFER_TESTS) $(IPBUS_TESTS)
	# Run all of the tests
	ls $^ | xargs -n 1 -I{} sh -c "./{}"

clean: 
	rm -rf *.exe *.dSYM

.PHONY: all clean
