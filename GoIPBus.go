// GoIPbus

// --------------------------------------------------------------------
// Author:
//      PhD. Esteve Farres Berenguer
//      C/ Can Planes, 6
//      ES17160 - Anglès - Girona
//      Catalonia - Spain
//
// Contact:
//      mobile: +34 659 17 59 69
//      web: https://plus.google.com/+EsteveFarresBerenguer/posts
//      email: esteve.farres@gmail.com
//
// --------------------------------------------------------------------

// Host interface implements according with the io standard package
// Implemented interfaces are:
// 		• Reader		=> Register => 3.2	Read transaction (Type ID = 0x0)
// 		• ReaderAt 		=> Read transaction with offset
// 		• ReaderFrom 	=> FIFO		=> 3.3	Non-incrementing read transaction (Type ID = 0x2)
// 		• Writer		=> Register => 3.4	Write transaction (Type ID = 0x1)
// 		• WriterAt 		=> Write transaction with offset
// 		• WriterTo 		=> FIFO 	=> 3.5	Non-incrementing write transaction (Type ID = 0x3)
// 		• Seeker		=> Seek sets the offset for the next Read or Write
//
// Because these interfaces and primitives wrap lower-level operations with
// various implementations, unless otherwise informed clients should not
// assume they are safe for parallel execution.

// IPbus is a simple, reliable, IP-based protocol for controlling hardware devices.
// It assumes the existence of a virtual bus with 32-bit word addressing and 32-bit data transfer.
// The choice of 32-bit data width is fixed in this protocol, though the target is free to ignore
// address or data lines if desired.
package goipbus

// Packages

import (
	"errors"
	"fmt"
)

// Implementation of IPbus protocol version 2.0
const IPbusProtocolVersion = 0x2

// 32-bit data transfer
type IPbusWord int32

// 32-bit word addressing
type BaseAddress int32

// 2 IPbus Packet Header
// The request and reply of an IPbus packet always begins with a 32-bit header.
// This header has been added in version 2.0 of the protocol in order to support
// the reliability mechanism.

// 31            28 ! 27   24 !	23	   16 ! 15           8 ! 7	   4    ! 3        0
// Protocol Version	! Rsvd.   ! Packet ID                  ! Byte-order ! Packet Type
// (4 bits)           (4 bits)      (16 bits)                (4 bits)     (4 bits)
//
// The status and resend request packets are only big endian.
type IPbusPacketHeader int32

// PacketID (16 bits)
type IPbusPacketID int16

// Byte-order (4 bits)
type IPbusByteOrder int8

// Packet Type (4 bits)
type IPbusPacketType int8

// The IPbus protocol defines three Packet Types:
// 		• Control packet (i.e. contains IPbus transactions)
// 		• Status packet
// 		• Re-send request packet
const (
	ControlPacket IPbusPacketType = 0x00
	StatusPacket                  = 0x01
	RequestPacket                 = 0x02
	// 0x3 – 0xf	n/a			Rsvd.
)

var packetID IPbusPacketID = 0

// 3	IPbus Control Packet
// An IPbus control packet is the concatenation of a control packet header
// (Packet Type 0x0) and one or many IPbus transactions. Clearly when using a
// packet-based transport protocol, there is a maximum size to an IPbus control
// request/response packet – the maximum transmission unit (MTU) of that protocol.
// As a consequence, it is illegal to make an IPbus control request which would
// result in a response longer than the path MTU (usually 1500 bytes).
// The reply to each IPbus control request packet must start with the same 32-bit packet header (i.e. same Packet Type and Packet ID values). All requests with non-zero ID received by a target must have consecutive ID values. Otherwise the packet header will be considered invalid and the packet will be silently dropped. A target will always accept control packets with ID value of 0.
// The IPbus-level reliability mechanism only works with non-zero packet IDs. For simplicity a packet ID of 0x0 can be used for non-reliable traffic at any time. However it should be noted that the simultaneous use of both forms of IPbus traffic with a single target will disrupt the reliability mechanism.
// In the following sub-sections the generic IPbus transaction header is described first, and then the individual transaction types are presented.
type IPbusControlPacket []int32

//	• Info Code (four bits at 3 → 0)
type IPbusInfoCode int8

//	• Info Code
//		o Defines the direction and error state of the transaction request/response.
// 		All requests (i.e. client to target) must have an Info Code of 0xf. All
// 		successful transaction responses must have an Info Code of 0x0.  All
// 		other values are transaction response error codes (or not yet specified by the protocol).
const (
	RequestHandledSuccesfully IPbusInfoCode = 0x00
	BadHeader                               = 0x01
	BusErrorOnRead                          = 0x04
	BusErrorOnWrite                         = 0x05
	BusTimeOutOnRead                        = 0x06
	BusTimeOutOnWrite                       = 0x07
	OutboundRequest                         = 0x0f
)

// 3.1	IPbus Transaction Header
// 31            28 ! 27		   16 ! 15           8 ! 7	   4 ! 3	0
// Protocol Version	! Transaction ID  ! Words (8 bits) ! Type ID ! Info Code
// (4 bits)            (12 bits)                         (4 bits)	(4 bits)
//The definition of the above fields is as follows:
//•	Protocol Version (four bits at 31 → 28)
//	o	Protocol version field – should be set to 2 for this version of the protocol.
//•	Transaction ID (twelve bits at 27 → 16)
//	o	Transaction identification number, so the client/target can track each transaction within a given packet.
//	o	Note that in order to support the possibility of using jumbo (9kB) Ethernet frames, the transaction ID number cannot be less than 12 bits wide so as to prevent the possibility of the transaction ID wrapping around within a single IPbus packet.
//•	Words (eight bits at 15 → 8)
//	o	Number of 32-bit words within the addressable memory space of the bus itself that are interacted with, i.e. written/read.
//	o	Defines read/write size of block reads/writes.
//	o	Reads/writes of size greater than 255 words must be split across two or more IPbus transactions.  Having a Words field that is only eight bits wide is a necessary trade-off required for keeping the transaction headers only 32 bits in size.
//•	Type ID (four bits at 7 → 4)
//	o	Defines the type (e.g. read/write) of the IPbus transaction – see the Transaction Types section for the different ID codes.
//•	Info Code (four bits at 3 → 0)
//	o	Defines the direction and error state of the transaction request/response. All requests (i.e. client to target) must have an Info Code of 0xf. All successful transaction responses must have an Info Code of 0x0.  All other values are transaction response error codes (or not yet specified by the protocol).
type IPbusTransactionHeader int32

// Transaction ID (12 bits)
type IPbusTransactionID int16

// Type ID (4 bits)
type IPbusTransactionTypeID int8

// Transaction Types
const (
	ReadTypeID                IPbusTransactionTypeID = 0x00 // 3.2	Read transaction (Type ID = 0x0)
	NonIncrementalReadTypeID                         = 0x02 // 3.3	Non-incrementing read transaction (Type ID = 0x2))
	WriteTypeID                                      = 0x01 // 3.4	Write transaction (Type ID = 0x1)
	NonIncrementalWriteTypeID                        = 0x03 // 3.5	Non-incrementing write transaction (Type ID = 0x3)
	RMWbitsTypeID                                    = 0x04 // 3.6	Read/Modify/Write bits
	RMWsumTypeID                                     = 0x05 // 3.7	Read/Modify/Write sum (RMWsum) transaction (Type ID = 0x5)
	ConfigurationSpaceRead                           = 0x06 // 3.8	Configuration space read transaction (Type ID = 0x6)
	ConfigurationSpaceWrite                          = 0x07 // 3.9	Configuration space write transaction (Type ID = 0x7)
)

// 4	IPbus Status Packet
// The status packet is new in this version of the protocol, and is necessary for
// the reliability mechanism, as well as being useful for debugging the target’s
// recent activity.  Unlike the control packet it is only big endian, little endian
// requests will be ignored.
type IPbusStatusPacket [16]int32

//5	IPbus Re-send Packet
//The re-send request packet is also a new addition of this protocol version and it is denoted by a Packet Type value of 0x3 in the packet header. This packet type is necessary for the reliability mechanism, and its purpose is to trigger a re-send of one of the target’s recent outbound control packets.  Like the status request, a re-send request packet must be big endian.
type IPbusResendPacket int32

//IPbusPacket
//IPbusClient
//IPbusTarget

// All requests with non-zero ID received by a target must have consecutive ID values. Otherwise the packet header will be considered invalid and the packet will be silently dropped. A target will always accept control packets with ID value of 0.
// The IPbus-level reliability mechanism only works with non-zero packet IDs. For simplicity a packet ID of 0x0 can be used for non-reliable traffic at any time. However it should be noted that the simultaneous use of both forms of IPbus traffic with a single target will disrupt the reliability mechanism.
var transactionID IPbusTransactionID = 0

// Note that the maximum size of a standard Ethernet packet (without using jumbo
// frames) is 1500 bytes; with an IP header of 20 bytes and a UDP header of 8 bytes,
// this gives the maximum IPbus packet size of 368 32-bit words, or 1472 bytes.
// Longer block transfers must be split at the software level into individual packets.
const maxWordSize int8 = 368
const maxByteSize int16 = 1472

// baseAddress store the address of the transaction
var baseAddress = 0

// transactionSize define the size of the transaction
var transactionSize = 0

// SectionReader implements Read, Seek, and ReadAt on a section
// of an underlying ReaderAt.
type SectionReader struct {
	r     ReaderAt
	base  int64
	off   int64
	limit int64
}

// Implementation of variables according with the io standard package

// EOF is the error returned by Read when no more input is available.
// Functions should return EOF only to signal a graceful end of input.
// If the EOF occurs unexpectedly in a structured data stream,
// the appropriate error is either ErrUnexpectedEOF or some other error
// giving more detail.
var EOF = errors.New("EOF")

// ErrUnexpectedEOF means that EOF was encountered in the
// middle of reading a fixed-size block or data structure.
var ErrUnexpectedEOF = errors.New("unexpected EOF")

// ErrNoProgress is returned by some clients of an io.Reader when
// many calls to Read have failed to return any data or error,
// usually the sign of a broken io.Reader implementation.
var ErrNoProgress = errors.New("multiple Read calls return no data or error")

// private package error variables
var errWhence = errors.New("Seek: invalid whence")
var errOffset = errors.New("Seek: invalid offset")

// Interfaces

// Reader is the interface that wraps the basic Read method.
//
// Read reads up to len(p) bytes into p.  It returns the number of bytes
// read (0 <= n <= len(p)) and any error encountered.  Even if Read
// returns n < len(p), it may use all of p as scratch space during the call.
// If some data is available but not len(p) bytes, Read conventionally
// returns what is available instead of waiting for more.
//
// When Read encounters an error or end-of-file condition after
// successfully reading n > 0 bytes, it returns the number of
// bytes read.  It may return the (non-nil) error from the same call
// or return the error (and n == 0) from a subsequent call.
// An instance of this general case is that a Reader returning
// a non-zero number of bytes at the end of the input stream may
// return either err == EOF or err == nil.  The next Read should
// return 0, EOF.
//
// Callers should always process the n > 0 bytes returned before
// considering the error err.  Doing so correctly handles I/O errors
// that happen after reading some bytes and also both of the
// allowed EOF behaviors.
//
// Implementations of Read are discouraged from returning a
// zero byte count with a nil error, except when len(p) == 0.
// Callers should treat a return of 0 and nil as indicating that
// nothing happened; in particular it does not indicate EOF.
//
// Implementations must not retain p.
type Reader interface {
	Read(p []byte) (n int, err error)
}

// Writer is the interface that wraps the basic Write method.
//
// Write writes len(p) bytes from p to the underlying data stream.
// It returns the number of bytes written from p (0 <= n <= len(p))
// and any error encountered that caused the write to stop early.
// Write must return a non-nil error if it returns n < len(p).
// Write must not modify the slice data, even temporarily.
//
// Implementations must not retain p.
type Writer interface {
	Write(p []byte) (n int, err error)
}

// Closer is the interface that wraps the basic Close method.
//
// The behavior of Close after the first call is undefined.
// Specific implementations may document their own behavior.
type Closer interface {
	Close() error
}

// Seeker is the interface that wraps the basic Seek method.
//
// Seek sets the offset for the next Read or Write to offset,
// interpreted according to whence: 0 means relative to the origin of
// the file, 1 means relative to the current offset, and 2 means
// relative to the end.  Seek returns the new offset and an error, if
// any.
//
// Seeking to a negative offset is an error. Seeking to any positive
// offset is legal, but the behavior of subsequent I/O operations on
// the underlying object is implementation-dependent.
type Seeker interface {
	Seek(offset int64, whence int) (int64, error)
}

// ReadWriter is the interface that groups the basic Read and Write methods.
type ReadWriter interface {
	Reader
	Writer
}

// ReadSeeker is the interface that groups the basic Read and Seek methods.
type ReadSeeker interface {
	Reader
	Seeker
}

// WriteSeeker is the interface that groups the basic Write and Seek methods.
type WriteSeeker interface {
	Writer
	Seeker
}

// ReadWriteSeeker is the interface that groups the basic Read, Write and Seek methods.
type ReadWriteSeeker interface {
	Reader
	Writer
	Seeker
}

// ReaderFrom is the interface that wraps the ReadFrom method.
//
// ReadFrom reads data from r until EOF or error.
// The return value n is the number of bytes read.
// Any error except io.EOF encountered during the read is also returned.
//
// The Copy function uses ReaderFrom if available.
type ReaderFrom interface {
	ReadFrom(r Reader) (n int64, err error)
}

// WriterTo is the interface that wraps the WriteTo method.
//
// WriteTo writes data to w until there's no more data to write or
// when an error occurs. The return value n is the number of bytes
// written. Any error encountered during the write is also returned.
//
// The Copy function uses WriterTo if available.
type WriterTo interface {
	WriteTo(w Writer) (n int64, err error)
}

// ReaderAt is the interface that wraps the basic ReadAt method.
//
// ReadAt reads len(p) bytes into p starting at offset off in the
// underlying input source.  It returns the number of bytes
// read (0 <= n <= len(p)) and any error encountered.
//
// When ReadAt returns n < len(p), it returns a non-nil error
// explaining why more bytes were not returned.  In this respect,
// ReadAt is stricter than Read.
//
// Even if ReadAt returns n < len(p), it may use all of p as scratch
// space during the call.  If some data is available but not len(p) bytes,
// ReadAt blocks until either all the data is available or an error occurs.
// In this respect ReadAt is different from Read.
//
// If the n = len(p) bytes returned by ReadAt are at the end of the
// input source, ReadAt may return either err == EOF or err == nil.
//
// If ReadAt is reading from an input source with a seek offset,
// ReadAt should not affect nor be affected by the underlying
// seek offset.
//
// Clients of ReadAt can execute parallel ReadAt calls on the
// same input source.
//
// Implementations must not retain p.
type ReaderAt interface {
	ReadAt(p []byte, off int64) (n int, err error)
}

// WriterAt is the interface that wraps the basic WriteAt method.
//
// WriteAt writes len(p) bytes from p to the underlying data stream
// at offset off.  It returns the number of bytes written from p (0 <= n <= len(p))
// and any error encountered that caused the write to stop early.
// WriteAt must return a non-nil error if it returns n < len(p).
//
// If WriteAt is writing to a destination with a seek offset,
// WriteAt should not affect nor be affected by the underlying
// seek offset.
//
// Clients of WriteAt can execute parallel WriteAt calls on the same
// destination if the ranges do not overlap.
//
// Implementations must not retain p.
type WriterAt interface {
	WriteAt(p []byte, off int64) (n int, err error)
}

// A LimitedReader reads from R but limits the amount of
// data returned to just N bytes. Each call to Read
// updates N to reflect the new amount remaining.
type LimitedReader struct {
	R Reader // underlying reader
	N int64  // max bytes remaining
}

func resetTransactionID() error {
	transactionID = 0
	return nil
}

func resetPacketID() error {
	packetID = 0
	return nil
}

// Packet ID (16 bits) [0x0 , 0xffff]
func increasePacketID() (n IPbusPacketID, err error) {
	if packetID == 0xffff {
		packetID = 0
	} else {
		packetID++
	}
	return packetID, nil
}

// Transaction ID (12 bits) [0x0 , 0x0fff]
func increaseTransactionID() (err error) {
	if transactionID == 0x0fff {
		transactionID = 0
	} else {
		transactionID++
	}
	return nil
}

// Build a IPbus transaction header
func packetHeader(byteOrder IPbusByteOrder, packetType IpbusPacketType) (header IPbusPacketHeader, err error) {
	var word int32 = IPbusProtocolVersion << 30
	word = word & ((transactionType << 4) & packetType)
	word = word & packetID << 16
	err := increasePacketID()
	if err != nil {
		panic()
	}
	return word, err
}

// Build a IPbus transaction header
func transactionHeader(transactionType IPbusTransactionTypeID, size int8) (header IPbusPacketHeader, err error) {
	var word int32 = IPbusProtocolVersion << 30
	word = word & ((transactionType << 4) & OutboundRequest)
	word = word & (size << 8)
	word = word & transactionID << 16
	err := increaseTransactionID()
	if err != nil {
		panic()
	}
	return word, err
}

func readHeader(addr BaseAddres, size int8) (word0 IPbusPacketHeader, err error) {
	word0, err := transactionHeader(ReadTypeID, size)
	return word0, err
}

func writeHeader(addr BaseAddres, word []int32) (word0 IPbusPacketHeader, err error) {
	size := size(data)
	word0, err := transactionHeader(WriteTypeID, size)
	return word0, err
}

func NonIncrementalReadHeader(addr BaseAddres, size int8) (word0 IPbusPacketHeader, err error) {
	word0, err := transactionHeader(NonIncrementalReadTypeID, size)
	return word0, err
}

func NonIncrementalWriteHeader(addr BaseAddres, word []int32) (word0 IPbusPacketHeader, err error) {
	size := size(data)
	word0, err := transactionHeader(NonIncrementalWriteTypeID, size)
	return word0, err
}

func setBaseAddress(addr BaseAddress) error {
	baseAddress = addr
	return nil
}

func setTransactionSize(size int8) error {
	transactionSize = size
	return nil
}

func Read(p []byte) (n int, err error) {
	word0, err := transactionHeader(ReadTypeID, size)
	return word0, err
}

func Seek(offset int64, whence int) (int64, error) {

}

func Write(p []byte) (nn int, err error) {
	word0, err := transactionHeader(ReadTypeID, size)
	return word0, err
}

func WriteAt(p []byte, off int64) (n int, err error) {

}

func WriteTo(w Writer) (n int64, err error) {

}
