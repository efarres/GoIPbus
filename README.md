# GoIPbus
Go implementation of the IPbus a simple, reliable, IP-based protocol for controlling hardware devices.
Initial versions are oriented to "Out of Band" systems.

IPbus is a flexible Ethernet-based control system. IPbus is developed by the CERN as control system for xTCA hardware.
The CERN IPbus suite implements a reliable high-performance control link for particle physics electronics. The GoIPBus project tray to reuse the protocol to control hardware devices.

Protocol spec is available at [pbus_protocol_v2_0](https://svnweb.cern.ch/trac/cactus/browser/trunk/doc/ipbus_protocol_v2_0.pdf)

It assumes the existence of a virtual bus with 32-bit word addressing and 32-bit data transfer. The choice of 32-bit data width is fixed in this protocol, though the target is free to ignore address or data lines if desired.

GoIPbus map IPBus transactions to Go IO standard package interfaces.

Implemented Interfaces
-	Read <=> 3.2	Read transaction (Type ID = 0x0)
-	ReadAT  <=> 3.3	Non-incrementing read transaction (Type ID = 0x2))
-	Write <=> 3.4	Write transaction (Type ID = 0x1)
-	WriteAt <=> 3.5	Non-incrementing write transaction (Type ID = 0x3)

ToDo, mapping of the IPbus interfaces.
-	RMWbitsTypeID  <=> 3.6	Read/Modify/Write bits
-	RMWsumTypeID  <=> 3.7	Read/Modify/Write sum (RMWsum) transaction (Type ID = 0x5)
-	ConfigurationSpaceRead  <=> 3.8	Configuration space read transaction (Type ID = 0x6)
-	ConfigurationSpaceWrite  <=> 3.9	Configuration space write transaction (Type ID = 0x7)
