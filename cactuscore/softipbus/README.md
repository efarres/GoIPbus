Soft IPBus
==========

An implementation of the [IPBus protocol](https://svnweb.cern.ch/trac/cactus/export/20772/trunk/doc/ipbus_protocol_v2_0.pdf) 
for "soft" target devices.  

Requirements
------------

For cross-compiling to the microblaze architecture, one must download and setup
the Xilinx Petalinux SDK.  

The [μHAL](https://svnweb.cern.ch/trac/cactus/wiki) client software is required
for some of the tests, and can be installed via by following [these
instructions](https://svnweb.cern.ch/trac/cactus/wiki/uhalQuickTutorial#HowtoInstalltheIPbusSuite).
You'll need to add the following to your environment:

```shell
export LD_LIBRARY_PATH=/opt/cactus/lib:$LD_LIBRARY_PATH
export PATH=/opt/cactus/bin:$PATH
```

Building
--------

To build the server, you can just run:
```shell
make
````
which should produce the server binary ``bin/softipus``.  If the Petalinux 
environement is setup (i.e. ``$PETALINUX`` is defined), it will be automatically
cross-compiled.  There are some additional targets (romfs, image) which exist
to support integration into the Petalinux ROM building GUI.

Testing
-------

The unit tests are run on the local architecture by running ``make unittest``.  A
full scale integration test of memory reading/writing using the uHAL can be run
with ``make test``.

Running on the CTP6
-------------------

You can scp /bin/softipbus(-forward) to the /tmp directory on the CTP6, and run
the server on the CTP6 back-end Petalinux by executing:

```shell
$ /tmp/softipbus
```

To serve IPBus, forwarding the packets over the UART to the front-end, one
should execute:

```shell
/tmp/softipbus-forward
```

which by default will forward over the TTY defined at /dev/ttyUL1.  The
forwarding TTY can be configured in the Makefile.

Miscellany
----------

Tunneling access to IPBus TCP port on the CTP6 back-end.

```shell
# from outside ayinger
ssh -L 60002:root@192.168.1.31:60002 ayinger.hep.wisc.edu
# on ayinger
ssh -L 60002:0.0.0.0:60002 root@192.168.1.31
```
