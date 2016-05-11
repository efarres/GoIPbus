# Cross-compiling to a Petalinux target
ifdef PETALINUX

# FIXME - workaround until we configure things correctly at 904
#include $(PETALINUX)/software/petalinux-dist/tools/user-commons.mk

PETADIST=$(PETALINUX)/software/petalinux-dist

CC=$(PETALINUX)/tools/linux-i386/microblazeel-xilinx-linux-gnu/bin/microblazeel-xilinx-linux-gnu-gcc

LDFLAGS=-I$(PETADIST)/stage/include \
	 -I$(PETADIST)/stage/usr/include \
	 -I$(PETADIST)/stage/usr/local/include \
	 -I$(PETADIST) -I$(PETADIST)/include \
	 -L$(PETADIST)/lib -L$(PETADIST)/stage/lib \
	 -L$(PETADIST)/stage/usr/lib -L$(PETADIST)/stage/usr/local/lib


CFLAGS=-fomit-frame-pointer -pipe -fno-common -fno-builtin \
       -mxl-multiply-high -mno-xl-soft-mul -mno-xl-soft-div \
       -mxl-barrel-shift -mxl-pattern-compare -mhard-float -mcpu=v8.40.b \
       -Wall -Werror -std=gnu99 \
       -DEMBED -Dlinux -D__linux__ -Dunix -fPIC -DLINUX

endif



# ############################################################################
# #####    Installation specific options                  ####################
# ############################################################################

# Serial TTY to forward packets to another device
ifndef CONFIG_USER_APPS_SOFTIPBUS_IPBUS_FORWARD_TX
  CONFIG_USER_APPS_SOFTIPBUS_IPBUS_FORWARD_TX=/dev/ttyUL1
endif
# Serial TTY to receive packets from another device
ifndef CONFIG_USER_APPS_SOFTIPBUS_IPBUS_FORWARD_RX
  CONFIG_USER_APPS_SOFTIPBUS_IPBUS_FORWARD_RX=/dev/ttyUL1
endif
# TCP port for forwarding server
ifndef CONFIG_USER_APPS_SOFTIPBUS_IPBUS_FORWARD_PORT
  CONFIG_USER_APPS_SOFTIPBUS_IPBUS_FORWARD_PORT=60001
endif
# TCP port for local server
ifndef CONFIG_USER_APPS_SOFTIPBUS_IPBUS_LOCAL_PORT
  CONFIG_USER_APPS_SOFTIPBUS_IPBUS_LOCAL_PORT=60002
endif

# Logging level: 0-suppress, 1-error, 2-info, 3-debug
ifndef LOG_LEVEL
LOG_LEVEL=0x0
endif

CFLAGS += -ggdb -std=gnu99 -Wall -Werror -DLOG_LEVEL=$(LOG_LEVEL)
SHAREDOPTS = -fPIC -shared
LDFLAGS += -Iinclude

# Definitions of source units
BUFFER_SRCS=src/bytebuffer.c src/circular_buffer.c src/buffer.c
CORE_SRCS=src/serialization.c src/packethandler.c $(BUFFER_SRCS)
LOCAL_SRV_SRCS=src/transactionhandler.c src/handlers.c
FORWARDING_SRCS=src/forwardingtransactionhandler.c \
		src/nullmembase.c src/tty_functions.c

everything: all test

all: lib/buffers.so bin/softipbus bin/softipbus-forward bin/softipbus-test

# Common buffer library
lib/buffers.so: $(BUFFER_SRCS)
	mkdir -p lib
	$(CC) $(LDFLAGS) $(CFLAGS) $(SHAREDOPTS) -o $@ $(BUFFER_SRCS)

# TCP IPBus server accessing local memory (direct memory access)
bin/softipbus: $(CORE_SRCS) $(LOCAL_SRV_SRCS) src/nullmembase.c src/serve-tcp.c
	mkdir -p bin
	$(CC) $(LDFLAGS) $(CFLAGS) -DPORT="$(CONFIG_USER_APPS_SOFTIPBUS_IPBUS_LOCAL_PORT)" -o $@ $^

# TCP IPBus server forwarding packets across a TTY
bin/softipbus-forward: $(CORE_SRCS) $(FORWARDING_SRCS) src/nullmembase.c src/serve-tcp.c
	mkdir -p bin
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^\
	  -DPORT=$(CONFIG_USER_APPS_SOFTIPBUS_IPBUS_FORWARD_PORT) \
	  -DIPBUS_FORWARD_TX=$(CONFIG_USER_APPS_SOFTIPBUS_IPBUS_FORWARD_TX) \
	  -DIPBUS_FORWARD_RX=$(CONFIG_USER_APPS_SOFTIPBUS_IPBUS_FORWARD_RX)

# A version which uses user-creatable fake TTYs
bin/softipbus-forward-test: $(CORE_SRCS) $(FORWARDING_SRCS) src/nullmembase.c src/serve-tcp.c
	mkdir -p bin
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^\
	  -DPORT=$(CONFIG_USER_APPS_SOFTIPBUS_IPBUS_FORWARD_PORT) \
	  -DIPBUS_FORWARD_TX=/tmp/test_ipbus_tx_fifo \
	  -DIPBUS_FORWARD_RX=/tmp/test_ipbus_rx_fifo

# TCP IPBus server accessing allocated test memory
bin/softipbus-test: $(CORE_SRCS) $(LOCAL_SRV_SRCS) src/testmembase.c src/serve-tcp.c
	mkdir -p bin
	$(CC) $(LDFLAGS) $(CFLAGS) -DPORT=$(CONFIG_USER_APPS_SOFTIPBUS_IPBUS_LOCAL_PORT) -o $@ $^

# TTY IPBus server accessing allocated test memory
bin/softipbus-serial: $(CORE_SRCS) $(LOCAL_SRV_SRCS) src/testmembase.c src/serve-serial.c
	mkdir -p bin
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^

unittest:
	cd tests/ && LOG_LEVEL="$(LOG_LEVEL)" make all

test: unittest bin/softipbus-test bin/softipbus-forward-test bin/softipbus-serial all
ifdef PETALINUX
	@echo "\$$PETALINUX is defined, so we are cross compiling."
	@echo "Unset \$$PETALINUX if you want to run the integration tests."
else
	echo "Testing R/W over TCP"
	IPBUS_LOCAL=1 python tests/test_server.py
	echo "Testing R/W over a TTY"
	python tests/test_forwarding.py
endif

clean:
	rm -fr bin/* lib/*
	cd tests/ && make clean


# Make sure our binary is up to date before uploading
.upload2ctp: bin/softipbus-forward
	touch $@

upload2ctp: .upload2ctp
	scp bin/softipbus* root@$(CTPIP):/tmp

forwardonctp: .upload2ctp
	# Connects to CTP, forwarding the softipbus-forward port,
	# and then executes the program uploaded by upload2ctp.
	ssh -L 60001:0.0.0.0:60001 root@$(CTPIP) "killall softipbus-forward; /tmp/softipbus-forward"

cleanonctp: .upload2ctp
	# Connects to CTP, forwarding the softipbus-forward port,
	# and then executes the program uploaded by upload2ctp.
	ssh root@$(CTPIP) "killall softipbus-forward"

# Petalinux built-in application configuration
# Adds functionality for compiling during the kernel build

# Optionally strip the final file
ifndef CONFIG_USER_DEBUG
DO_STRIP=do_strip
endif

do_strip: all
	$(STRIP) bin/softipbus
	$(STRIP) bin/softipbus-forward
	$(STRIP) bin/softipbus-forward-test
	$(STRIP) bin/softipbus-test

romfs: $(DO_STRIP) all
ifeq ($(CONFIG_USER_APPS_SOFTIPBUS_IPBUS_LOCAL_ENABLED), 1)
	$(ROMFSINST) -d bin/softipbus /bin/softipbus
endif
ifeq ($(CONFIG_USER_APPS_SOFTIPBUS_IPBUS_FORWARD_ENABLED), 1)
	$(ROMFSINST) -d bin/softipbus-forward /bin/softipbus-forward
endif
	$(ROMFSINST) -d bin/softipbus-test /bin/softipbus-test
	$(ROMFSINST) -d -p 0755 etc/init.d-softipbus /etc/init.d/softipbus
	$(ROMFSINST) -s /etc/init.d/softipbus /etc/rc.d/S50softipbus

image: romfs
	make -C ${PETALINUX}/software/petalinux-dist image

.PHONY: all clean test unittest strip romfs image
